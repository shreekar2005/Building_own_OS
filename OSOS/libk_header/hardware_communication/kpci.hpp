#ifndef _OSOS_HARDWARECOMMUNICATION_KPCI_H
#define _OSOS_HARDWARECOMMUNICATION_KPCI_H

#include <cstdint>
#include "hardware_communication/kport.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "driver/kdriver.hpp"



namespace hardware_communication
{
    enum BaseAddressRegisterType{
        memoryMapping = 0,
        inputOutput = 1
    };
    
    class BaseAddressRegister{
        public:
            bool prefetchable;
            uint8_t* address;
            uint32_t size;
            BaseAddressRegisterType type;
    };

    class PCI_DeviceDescriptor{
        public:
            uint8_t bus;
            uint8_t device;
            uint8_t function;

            uint16_t vendorId;
            uint16_t deviceId;

            uint8_t revision;
            uint8_t interfaceId;
            uint8_t subclassId;
            uint8_t classId;
            
            uint8_t headerType;
            
            uint32_t bar[6];
            
            uint8_t interrupt;

            uint32_t portBase[6];

            PCI_DeviceDescriptor();
            ~PCI_DeviceDescriptor();
    };

    class PCI_Controller{
        private:
            Port32Bit dataPort;
            Port32Bit commandPort;

            uint32_t getAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);

        public:
            PCI_Controller();
            ~PCI_Controller();

            uint32_t read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);
            uint16_t read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);
            uint8_t read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);

            void write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint32_t value);
            void write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint16_t value);
            void write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint8_t value);
            
            bool deviceHasFunctions(uint8_t bus, uint8_t device);
            PCI_DeviceDescriptor getDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function);

            void recursiveSelDriver(uint8_t bus, driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager);
            void selectDrivers(driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager);

            driver::Driver* getDriver(PCI_DeviceDescriptor dev, hardware_communication::InterruptManager* interrupt_manager);
            BaseAddressRegister getBaseAddressRegister(uint8_t bus, uint8_t device, uint8_t function, uint16_t bar);

            void scanBus(uint8_t bus);

            
    };
}

#endif

