#ifndef _OSOS_HARDWARECOMMUNICATION_KPCI_H
#define _OSOS_HARDWARECOMMUNICATION_KPCI_H

#include <cstdint>
#include "hardware_communication/kport.hpp"
#include "driver/kdriver.hpp"
#include "basic/kiostream.hpp"

namespace hardware_communication
{
    class PCI_DeviceDescriptor{
        public:
            uint8_t bus;
            uint8_t device;
            uint8_t function;

            uint16_t vendorId;
            uint16_t deviceId;

            uint8_t classId;
            uint8_t subclassId;
            uint8_t interfaceId;
            uint8_t revision;
            
            uint8_t headerType;
            uint8_t interrupt;

            uint32_t bar0;
            uint32_t bar1;
            uint32_t bar2;
            uint32_t bar3;
            uint32_t bar4;
            uint32_t bar5;

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
            void scanBus(uint8_t bus, driver::DriverManager* driver_manager);
            void selectDrivers(driver::DriverManager* driver_manager);
    };
}

#endif

