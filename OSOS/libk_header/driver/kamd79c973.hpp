#ifndef KAMD79C973_HPP
#define KAMD79C973_HPP

#include "essential/ktypes.hpp"
#include "driver/kdriver.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"
#include "hardware_communication/kpci.hpp"

namespace driver {

    class amd_am79c973 : public Driver, public hardware_communication::InterruptHandler {
        
        hardware_communication::Port16Bit MACAddress0Port;
        hardware_communication::Port16Bit MACAddress2Port;
        hardware_communication::Port16Bit MACAddress4Port;
        hardware_communication::Port16Bit RegisterDataPort;
        hardware_communication::Port16Bit RegisterAddressPort;
        hardware_communication::Port16Bit ResetPort;
        hardware_communication::Port16Bit BusControlRegisterDataPort;

        // Corrected struct with explicit types (No Bitfields)
        struct InitializationBlock {
            uint16_t mode;
            uint16_t lengthInfo; // Replaces bitfields
            uint32_t physicalAddressLow;
            uint16_t physicalAddressHigh;
            uint16_t reserved;
            uint64_t logicalAddress;
            uint32_t recvBufferDescrAddress;
            uint32_t sendBufferDescrAddress;
        } __attribute__((packed));

        // Added 'volatile' to ensure compiler doesn't cache these values
        struct BufferDescriptor {
            volatile uint32_t address;
            volatile uint32_t flags;
            volatile uint32_t flags2;
            volatile uint32_t avail;
        } __attribute__((packed));

        InitializationBlock* initBlock;
        BufferDescriptor* sendBufferDescr;
        BufferDescriptor* recvBufferDescr;

        // NOTE: We REMOVED the arrays (sendBuffers, etc.) from here.
        // They are now static variables in the .cpp file to ensure 
        // they live in Identity Mapped memory (BSS).

        uint8_t currentSendBuffer;
        uint8_t currentRecvBuffer;

    public:
        amd_am79c973(hardware_communication::PCI_DeviceDescriptor* dev, 
                        hardware_communication::InterruptManager* interrupts);
        ~amd_am79c973();

        void activate();
        int reset();
        void deactivate();
        void Send(uint8_t* buffer, int count);
        uint32_t handleInterrupt(uint32_t esp);
    };
}

#endif