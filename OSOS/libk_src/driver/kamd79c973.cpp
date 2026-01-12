#include "driver/kamd79c973.hpp"
#include "basic/kiostream.hpp"

using namespace basic;
using namespace driver;
using namespace hardware_communication;

amd_am79c973::amd_am79c973(PCI_DeviceDescriptor* dev, InterruptManager* interrupts)
    : Driver(),
      InterruptHandler(dev->interrupt + 0x20, interrupts),
      MACAddress0Port(dev->portBase[0]),
      MACAddress2Port(dev->portBase[0] + 0x02),
      MACAddress4Port(dev->portBase[0] + 0x04),
      RegisterDataPort(dev->portBase[0] + 0x10),
      RegisterAddressPort(dev->portBase[0] + 0x12),
      ResetPort(dev->portBase[0] + 0x14),
      BusControlRegisterDataPort(dev->portBase[0] + 0x16)
{
    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    // --- 1. SETUP SEND DESCRIPTOR RING ---
    // Align the pointer for the Descriptors (Structs)
    uint32_t raw_send_descr_ptr = (uint32_t)sendBufferDescrMemory;
    sendBufferDescr = (BufferDescriptor*)((raw_send_descr_ptr + 15) & ~0xF);

    // Initialize the Descriptors to point to the Buffers
    for(uint8_t i = 0; i < 8; i++) {
        // Align the pointer for the actual Buffer Data
        uint32_t raw_buf_ptr = (uint32_t)sendBuffers[i];
        
        sendBufferDescr[i].address = (raw_buf_ptr + 15 ) & ~0xF;
        sendBufferDescr[i].flags = 0x7FF | 0xF000;
        sendBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avail = 0;
    }

    // --- 2. SETUP RECEIVE DESCRIPTOR RING ---
    // Align the pointer for the Descriptors (Structs)
    uint32_t raw_recv_descr_ptr = (uint32_t)recvBufferDescrMemory;
    recvBufferDescr = (BufferDescriptor*)((raw_recv_descr_ptr + 15) & ~0xF);

    // Initialize the Descriptors to point to the Buffers
    for(uint8_t i = 0; i < 8; i++) {
        // Align the pointer for the actual Buffer Data
        uint32_t raw_buf_ptr = (uint32_t)recvBuffers[i];
        
        recvBufferDescr[i].address = (raw_buf_ptr + 15 ) & ~0xF;
        recvBufferDescr[i].flags = 0xF7FF | 0x80000000; // 0x80000000 = Owned by Card
        recvBufferDescr[i].flags2 = 0;
        recvBufferDescr[i].avail = 0;
    }

    // --- 3. SETUP INITIALIZATION BLOCK ---
    uint32_t raw_init_ptr = (uint32_t)initBlockMem;
    initBlock = (InitializationBlock*)((raw_init_ptr + 15) & ~0xF);

    initBlock->mode = 0; 
    initBlock->reserved1 = 0;
    initBlock->numSendBuffers = 3; // 3 means 8 buffers (2^3)
    initBlock->reserved2 = 0;
    initBlock->numRecvBuffers = 3; // 3 means 8 buffers (2^3)
    
    // Read MAC Address
    uint64_t mac0 = MACAddress0Port.read() % 256;
    uint64_t mac1 = MACAddress0Port.read() / 256;
    uint64_t mac2 = MACAddress2Port.read() % 256;
    uint64_t mac3 = MACAddress2Port.read() / 256;
    uint64_t mac4 = MACAddress4Port.read() % 256;
    uint64_t mac5 = MACAddress4Port.read() / 256;
    
    initBlock->physicalAddress = mac0 | (mac1 << 8) | (mac2 << 16) | (mac3 << 24) | (mac4 << 32) | (mac5 << 40);
    initBlock->reserved3 = 0;
    initBlock->logicalAddress = 0;
    initBlock->recvBufferDescrAddress = (uint32_t)recvBufferDescr;
    initBlock->sendBufferDescrAddress = (uint32_t)sendBufferDescr;
}

amd_am79c973::~amd_am79c973() {
}

void amd_am79c973::activate() {
    // Send Init Block Address
    RegisterAddressPort.write(1);
    RegisterDataPort.write((uint32_t)initBlock & 0xFFFF);
    RegisterAddressPort.write(2);
    RegisterDataPort.write(((uint32_t)initBlock >> 16) & 0xFFFF);
    
    // Set INIT and START
    RegisterAddressPort.write(0);
    RegisterDataPort.write(0x41); 

    // Enable Auto Padding (ASEL)
    RegisterAddressPort.write(4);
    uint32_t temp = RegisterDataPort.read();
    RegisterAddressPort.write(4);
    RegisterDataPort.write(temp | 0xC00);

    // Start the device
    RegisterAddressPort.write(0);
    RegisterDataPort.write(0x42);
}

int amd_am79c973::reset() {
    ResetPort.read();
    ResetPort.write(0);
    return 10;
}

void amd_am79c973::deactivate() {
    // TODO: Stop the device by writing to CSR0 if needed
}

void amd_am79c973::Send(uint8_t* buffer, int count) {
    int sendDescriptor = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1) % 8;

    if (count > 1518) count = 1518; // Ethernet MTU

    // Copy data to the pre-allocated send buffer
    // Note: We use the address stored in the descriptor, which we set up in the constructor
    uint8_t* dst = (uint8_t*)(sendBufferDescr[sendDescriptor].address);
    for(int i = 0; i < count; i++) {
        dst[i] = buffer[i];
    }

    // Setup Flags
    // 0xF000 in flags2 is required. 2's complement of count goes in lower bits.
    sendBufferDescr[sendDescriptor].flags2 = (uint32_t)(-count) | 0xF000;
    
    // OWN (0x80000000) = Card, STP (0x02000000) = Start, ENP (0x01000000) = End
    sendBufferDescr[sendDescriptor].flags = 0x8300F000;

    // Trigger Transmit Demand (TDMD) in CSR0
    RegisterAddressPort.write(0);
    RegisterDataPort.write(0x48); 
    
    printf("SENDING PACKET...\n");
}

uint32_t amd_am79c973::handleInterrupt(uint32_t esp) {
    RegisterAddressPort.write(0);
    uint32_t temp = RegisterDataPort.read();

    if((temp & 0x8000) == 0x8000) printf("AMD am79c973 ERROR\n");
    if((temp & 0x2000) == 0x2000) printf("AMD am79c973 COLLISION ERROR\n");
    if((temp & 0x1000) == 0x1000) printf("AMD am79c973 MISSED FRAME\n");
    if((temp & 0x0800) == 0x0800) printf("AMD am79c973 MEMORY ERROR\n");
    
    if((temp & 0x0200) == 0x0200) printf("AMD am79c973 DATA SENT\n");

    // Check for Receive Interrupt (RINT)
    if((temp & 0x0400) == 0x0400) 
    {
        printf("AMD am79c973 DATA RECEIVED\n");

        // Loop through all buffers that the card has released (OWN bit is 0)
        while((recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0)
        {
            // The buffer now contains data.
            // uint8_t* buffer = (uint8_t*)(recvBufferDescr[currentRecvBuffer].address);
            // int size = recvBufferDescr[currentRecvBuffer].flags2 & 0xFFF;
            
            // TODO: Pass 'buffer' and 'size' to your Ethernet layer
            printf(" Packet handled at buffer index: ");
            // printfHex(currentRecvBuffer); // Assuming you have a hex printer
            printf("\n");

            // Hand the buffer back to the card
            recvBufferDescr[currentRecvBuffer].flags2 = 0;
            recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF; // OWN = 1, Size = 2048

            currentRecvBuffer = (currentRecvBuffer + 1) % 8;
        }
    }

    // Acknowledge the interrupt
    RegisterAddressPort.write(0);
    RegisterDataPort.write(temp);

    if((temp & 0x0100) == 0x0100) printf("AMD am79c973 INIT DONE\n");

    return esp;
}