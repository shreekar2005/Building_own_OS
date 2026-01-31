#include "driver/kamd79c973.hpp"
#include "basic/kiostream.hpp"

using namespace basic;
using namespace driver;
using namespace hardware_communication;


RawDataHandler::RawDataHandler(amd_am79c973* backend)
{
    this->backend = backend;
    backend->setHandler(this);
}

RawDataHandler::~RawDataHandler()
{
    backend->setHandler(0);
}
            
bool RawDataHandler::onRawDataReceived(uint8_t* buffer, uint32_t size)
{
    (void) buffer;
    (void) size;
    return false;
}

void RawDataHandler::send(uint8_t* buffer, uint32_t size)
{
    backend->send(buffer, size);
}


amd_am79c973::amd_am79c973(hardware_communication::PCI_DeviceDescriptor *dev, hardware_communication::InterruptManager* interrupt_manager)
:   hardware_communication::InterruptHandler(dev->interrupt + 0x20, interrupt_manager),
    MACAddress0Port(dev->portBase[0]),
    MACAddress2Port(dev->portBase[0] + 0x02),
    MACAddress4Port(dev->portBase[0] + 0x04),
    registerDataPort(dev->portBase[0] + 0x10),
    registerAddressPort(dev->portBase[0] + 0x12),
    resetPort(dev->portBase[0] + 0x14),
    busControlRegisterDataPort(dev->portBase[0] + 0x16)
{
    this->handler = 0;
    currentSendBuffer = 0;
    currentRecvBuffer = 0;
    
    uint64_t MAC0 = MACAddress0Port.read() % 256;
    uint64_t MAC1 = MACAddress0Port.read() / 256;
    uint64_t MAC2 = MACAddress2Port.read() % 256;
    uint64_t MAC3 = MACAddress2Port.read() / 256;
    uint64_t MAC4 = MACAddress4Port.read() % 256;
    uint64_t MAC5 = MACAddress4Port.read() / 256;
    
    uint64_t MAC = MAC5 << 40
                 | MAC4 << 32
                 | MAC3 << 24
                 | MAC2 << 16
                 | MAC1 << 8
                 | MAC0;
    
    // 32 bit mode
    registerAddressPort.write(20);
    busControlRegisterDataPort.write(0x102);
    
    // STOP reset
    registerAddressPort.write(0);
    registerDataPort.write(0x04);
    
    // initBlock
    initBlock.mode = 0x0000; // promiscuous mode = false
    initBlock.reserved1 = 0;
    initBlock.numSendBuffers = 3;
    initBlock.reserved2 = 0;
    initBlock.numRecvBuffers = 3;
    initBlock.physicalAddress = MAC;
    initBlock.reserved3 = 0;
    initBlock.logicalAddress = 0;
    
    sendBufferDescr = (BufferDescriptor*)((((uint32_t)&sendBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.sendBufferDescrAddress = (uint32_t)sendBufferDescr;
    recvBufferDescr = (BufferDescriptor*)((((uint32_t)&recvBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.recvBufferDescrAddress = (uint32_t)recvBufferDescr;
    
    for(uint8_t i = 0; i < 8; i++)
    {
        sendBufferDescr[i].address = (((uint32_t)&sendBuffers[i]) + 15 ) & ~(uint32_t)0xF;
        sendBufferDescr[i].flags = 0x7FF
                                 | 0xF000;
        sendBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avail = 0;
        
        recvBufferDescr[i].address = (((uint32_t)&recvBuffers[i]) + 15 ) & ~(uint32_t)0xF;
        recvBufferDescr[i].flags = 0xF7FF
                                 | 0x80000000;
        recvBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avail = 0;
    }
    
    registerAddressPort.write(1);
    registerDataPort.write(  (uint32_t)(&initBlock) & 0xFFFF );
    registerAddressPort.write(2);
    registerDataPort.write(  ((uint32_t)(&initBlock) >> 16) & 0xFFFF );
    
}

amd_am79c973::~amd_am79c973(){}

void amd_am79c973::activate()
{
    registerAddressPort.write(0);
    registerDataPort.write(0x41);

    registerAddressPort.write(4);
    uint32_t temp = registerDataPort.read();
    registerAddressPort.write(4);
    registerDataPort.write(temp | 0xC00);
    
    registerAddressPort.write(0);
    registerDataPort.write(0x42);
}

int amd_am79c973::reset()
{
    resetPort.read();
    resetPort.write(0);
    return 10;
}

void amd_am79c973::deactivate(){}

uint32_t amd_am79c973::handleInterrupt(uint32_t esp)
{
    registerAddressPort.write(0);
    uint32_t temp = registerDataPort.read();
    
    if((temp & 0x8000) == 0x8000) printf("AMD am79c973 ERROR\n");
    if((temp & 0x2000) == 0x2000) printf("AMD am79c973 COLLISION ERROR\n");
    if((temp & 0x1000) == 0x1000) printf("AMD am79c973 MISSED FRAME\n");
    if((temp & 0x0800) == 0x0800) printf("AMD am79c973 MEMORY ERROR\n");
    if((temp & 0x0400) == 0x0400) receive();
    if((temp & 0x0200) == 0x0200) printf(" SENT");
                               
    // acknoledge
    registerAddressPort.write(0);
    registerDataPort.write(temp);
    
    if((temp & 0x0100) == 0x0100) printf("AMD am79c973 INIT DONE\n");
    
    return esp;
}

       
void amd_am79c973::send(uint8_t* buffer, int size)
{
    int sendDescriptor = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1) % 8;
    
    if(size > 1518) size = 1518;
    
    for(uint8_t *src = buffer + size -1,
                *dst = (uint8_t*)(sendBufferDescr[sendDescriptor].address + size -1);
                src >= buffer; src--, dst--) *dst = *src;
        
    printf("\nSEND: ");
    for(int i = 14+20; i < (size>64?64:size); i++)
    {
        printf("%x",buffer[i]);
    }
    
    sendBufferDescr[sendDescriptor].avail = 0;
    sendBufferDescr[sendDescriptor].flags2 = 0;
    sendBufferDescr[sendDescriptor].flags = 0x8300F000 | ((uint16_t)((-size) & 0xFFF));
    registerAddressPort.write(0);
    registerDataPort.write(0x48);
}

void amd_am79c973::receive()
{
    printf("\nRECV: ");
    
    for(; (recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0;
        currentRecvBuffer = (currentRecvBuffer + 1) % 8)
    {
        if(!(recvBufferDescr[currentRecvBuffer].flags & 0x40000000)
         && (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000) 
        
        {
            uint32_t size = recvBufferDescr[currentRecvBuffer].flags & 0xFFF;
            if(size > 64) // remove checksum
                size -= 4;
            
            uint8_t* buffer = (uint8_t*)(recvBufferDescr[currentRecvBuffer].address);

            for(uint32_t i = 14+20; i < (size>64?64:size); i++)
            {
                printf("%x",buffer[i]);
                printf(" ");
            }

            if(handler != 0)
                if(handler->onRawDataReceived(buffer, size))
                    send(buffer, size);
        }
        
        recvBufferDescr[currentRecvBuffer].flags2 = 0;
        recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;
    }
}

void amd_am79c973::setHandler(RawDataHandler* handler)
{
    this->handler = handler;
}

uint64_t amd_am79c973::getMACAddress()
{
    return initBlock.physicalAddress;
}

void amd_am79c973::setIPAddress(uint32_t ip)
{
    initBlock.logicalAddress = ip;
}

uint32_t amd_am79c973::getIPAddress()
{
    return initBlock.logicalAddress;
}