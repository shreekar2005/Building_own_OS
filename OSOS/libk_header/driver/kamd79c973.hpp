#ifndef _OSOS_DRIVER_KAMD79C973_H
#define _OSOS_DRIVER_KAMD79C973_H

#include "essential/ktypes.hpp"
#include "driver/kdriver.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"
#include "hardware_communication/kpci.hpp"

namespace driver {

    class amd_am79c973;
        
    class RawDataHandler
    {
    protected:
        amd_am79c973* backend;
    public:
        RawDataHandler(amd_am79c973* backend);
        ~RawDataHandler();
        
        virtual bool onRawDataReceived(uint8_t* buffer, uint32_t size);
        void send(uint8_t* buffer, uint32_t size);
    };

    class amd_am79c973 : public Driver, public hardware_communication::InterruptHandler {
        
        struct InitializationBlock
        {
            uint16_t mode;
            unsigned reserved1 : 4; // setting 4 bits as reserver
            unsigned numSendBuffers : 4;
            unsigned reserved2 : 4;
            unsigned numRecvBuffers : 4;
            uint64_t physicalAddress : 48; // setting 48 bits for physicalAddress (MAC Address)
            uint16_t reserved3;
            uint64_t logicalAddress;
            uint32_t recvBufferDescrAddress;
            uint32_t sendBufferDescrAddress;
        } __attribute__((packed));

        struct BufferDescriptor
        {
            uint32_t address;
            uint32_t flags;
            uint32_t flags2;
            uint32_t avail;
        } __attribute__((packed));

        hardware_communication::Port16Bit MACAddress0Port;
        hardware_communication::Port16Bit MACAddress2Port;
        hardware_communication::Port16Bit MACAddress4Port;
        hardware_communication::Port16Bit registerDataPort;
        hardware_communication::Port16Bit registerAddressPort;
        hardware_communication::Port16Bit resetPort;
        hardware_communication::Port16Bit busControlRegisterDataPort;

        InitializationBlock initBlock;

        BufferDescriptor* sendBufferDescr;
        uint8_t sendBufferDescrMemory[2048+15];
        uint8_t sendBuffers[2*1024+15][8]; // 8 buffers of each size 2 KB each (15bytes are for aligning buffers e.g. address of buffer should be multiple of 16)
        uint8_t currentSendBuffer;
        
        BufferDescriptor* recvBufferDescr;
        uint8_t recvBufferDescrMemory[2048+15];
        uint8_t recvBuffers[2*1024+15][8]; // same as sendBuffers
        uint8_t currentRecvBuffer;

        RawDataHandler* handler;

    public:
        amd_am79c973(hardware_communication::PCI_DeviceDescriptor* dev, 
                        hardware_communication::InterruptManager* interrupts);
        ~amd_am79c973();

        void activate();
        int reset();
        uint32_t handleInterrupt(uint32_t esp);
        void deactivate();

        void send(uint8_t* buffer, int count);
        void receive();
        void setHandler(RawDataHandler* handler);
        uint64_t getMACAddress();
        void setIPAddress(uint32_t);
        uint32_t getIPAddress();
    };
}

#endif