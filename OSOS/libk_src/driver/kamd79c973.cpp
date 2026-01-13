#include "driver/kamd79c973.hpp"
#include "basic/kiostream.hpp"

using namespace basic;
using namespace driver;
using namespace hardware_communication;

// --- STATIC MEMORY ALLOCATION (BSS SECTION) ---
// We move these out of the class to ensure they are in Identity Mapped memory.
// If they are on the Heap, the Virtual Address != Physical Address, causing DMA to fail.

static uint8_t bss_sendBufferDescrMemory[2048];
static uint8_t bss_recvBufferDescrMemory[2048];
static uint8_t bss_sendBuffers[8][2048 + 15];
static uint8_t bss_recvBuffers[8][2048 + 15];
static uint8_t bss_initBlockMem[256]; 


// Helper to unmask IRQ on PIC
void UnmaskIRQ(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21; // Master PIC Data Port
    } else {
        port = 0xA1; // Slave PIC Data Port
        irq -= 8;
    }

    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    value &= ~(1 << irq);
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

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
    // Unmask the specific IRQ for this device
    basic::printf("Network Card using IRQ %d\n", dev->interrupt);
    UnmaskIRQ(dev->interrupt);
    if(dev->interrupt >= 8) UnmaskIRQ(2); // Unmask Cascade if needed

    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    // SETUP SEND DESCRIPTOR RING (Using Static Memory)
    uint32_t raw_send_descr_ptr = (uint32_t)bss_sendBufferDescrMemory;
    sendBufferDescr = (BufferDescriptor*)((raw_send_descr_ptr + 15) & ~0xF);

    for(uint8_t i = 0; i < 8; i++) {
        uint32_t raw_buf_ptr = (uint32_t)bss_sendBuffers[i];
        
        sendBufferDescr[i].address = (raw_buf_ptr + 15 ) & ~0xF;
        sendBufferDescr[i].flags = 0x7FF | 0xF000;
        sendBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avail = 0;
    }

    // SETUP RECEIVE DESCRIPTOR RING (Using Static Memory)
    uint32_t raw_recv_descr_ptr = (uint32_t)bss_recvBufferDescrMemory;
    recvBufferDescr = (BufferDescriptor*)((raw_recv_descr_ptr + 15) & ~0xF);

    for(uint8_t i = 0; i < 8; i++) {
        uint32_t raw_buf_ptr = (uint32_t)bss_recvBuffers[i];
        
        recvBufferDescr[i].address = (raw_buf_ptr + 15 ) & ~0xF;
        recvBufferDescr[i].flags = 0xF7FF | 0x80000000;
        recvBufferDescr[i].flags2 = 0;
        recvBufferDescr[i].avail = 0;
    }

    // SETUP INITIALIZATION BLOCK (Using Static Memory)
    uint32_t raw_init_ptr = (uint32_t)bss_initBlockMem;
    initBlock = (InitializationBlock*)((raw_init_ptr + 15) & ~0xF);

    initBlock->mode = 0; 
    initBlock->lengthInfo = 0x3030; // 8 Buffers (Log2(8)=3) for RX and TX

    uint64_t mac0 = MACAddress0Port.read() % 256;
    uint64_t mac1 = MACAddress0Port.read() / 256;
    uint64_t mac2 = MACAddress2Port.read() % 256;
    uint64_t mac3 = MACAddress2Port.read() / 256;
    uint64_t mac4 = MACAddress4Port.read() % 256;
    uint64_t mac5 = MACAddress4Port.read() / 256;
    
    initBlock->physicalAddressLow = mac0 | (mac1 << 8) | (mac2 << 16) | (mac3 << 24);
    initBlock->physicalAddressHigh = mac4 | (mac5 << 8);
    initBlock->reserved = 0;
    initBlock->logicalAddress = 0;
    initBlock->recvBufferDescrAddress = (uint32_t)recvBufferDescr;
    initBlock->sendBufferDescrAddress = (uint32_t)sendBufferDescr;
}

amd_am79c973::~amd_am79c973() {
}

void amd_am79c973::activate() {
    // 1. Switch to 32-bit mode (SWSTYLE = 2)
    // This guarantees the card interprets our structs correctly.
    RegisterAddressPort.write(58);
    RegisterDataPort.write(0x0002);

    // 2. Write the Init Block Address
    RegisterAddressPort.write(1);
    RegisterDataPort.write((uint32_t)initBlock & 0xFFFF);
    RegisterAddressPort.write(2);
    RegisterDataPort.write(((uint32_t)initBlock >> 16) & 0xFFFF);
    
    // 3. Set INIT (0x01) and START (0x40) -> 0x41
    RegisterAddressPort.write(0);
    RegisterDataPort.write(0x41); 

    // 4. Poll for Init Done
    uint32_t temp = 0;
    while ((temp & 0x0100) == 0) {
        RegisterAddressPort.write(0);
        temp = RegisterDataPort.read();
    }
    
    // 5. Acknowledge Init Done
    RegisterAddressPort.write(0);
    RegisterDataPort.write(temp | 0x0100);

    // 6. Enable Auto Padding (ASEL)
    RegisterAddressPort.write(4);
    uint32_t temp2 = RegisterDataPort.read();
    RegisterAddressPort.write(4);
    RegisterDataPort.write(temp2 | 0xC00);

    // 7. Start the device (Interrupts Enable = 0x42)
    RegisterAddressPort.write(0);
    RegisterDataPort.write(0x42);
}

int amd_am79c973::reset() {
    ResetPort.read();
    ResetPort.write(0);
    return 10;
}

void amd_am79c973::deactivate() {
}

void amd_am79c973::Send(uint8_t* buffer, int count) {
    int sendDescriptor = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1) % 8;

    if (count > 1518) count = 1518; 

    uint8_t* dst = (uint8_t*)(sendBufferDescr[sendDescriptor].address);
    for(int i = 0; i < count; i++) {
        dst[i] = buffer[i];
    }

    uint16_t bcnt = (uint16_t)(-count);
    bcnt &= 0x0FFF;

    sendBufferDescr[sendDescriptor].flags = 0x8300F000 | bcnt;
    sendBufferDescr[sendDescriptor].flags2 = 0;

    RegisterAddressPort.write(0);
    RegisterDataPort.write(0x48); 
}

uint32_t amd_am79c973::handleInterrupt(uint32_t esp) {
    RegisterAddressPort.write(0);
    uint32_t temp = RegisterDataPort.read();

    if((temp & 0x0400) == 0x0400) 
    {
        while((recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0)
        {
             recvBufferDescr[currentRecvBuffer].flags2 = 0;
             recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF; 
             currentRecvBuffer = (currentRecvBuffer + 1) % 8;
        }
    }

    RegisterAddressPort.write(0);
    RegisterDataPort.write(temp);

    return esp;
}