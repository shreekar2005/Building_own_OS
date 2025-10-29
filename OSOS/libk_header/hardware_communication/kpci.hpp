#ifndef _OSOS_HARDWARECOMMUNICATION_KPCI_H
#define _OSOS_HARDWARECOMMUNICATION_KPCI_H

#include <cstdint>
#include "hardware_communication/kport.hpp"
#include "driver/kdriver.hpp"
#include "basic/kiostream.hpp"

namespace hardware_communication
{
    /// @brief Peripheral Component Interconnect Device Descriptor
    class PCI_DeviceDescriptor{
        public:
            uint32_t portBase;
            uint32_t interrupt;

            uint16_t bus;
            uint16_t device;
            uint16_t function;

            uint16_t vendorId;
            uint16_t deviceId;

            uint8_t classId;
            uint8_t subclassId;
            uint8_t interfaceId;

            uint8_t revision;

            PCI_DeviceDescriptor();
            ~PCI_DeviceDescriptor();
    };

    /// @brief Peripheral Component Interconnect Controller
    class PCI_Controller{
        private:
            Port32Bit dataPort;
            Port32Bit commandPort;
        public:
            PCI_Controller();
            ~PCI_Controller();
            uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset);
            void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset, uint32_t value);
            bool deviceHasFunctions(uint16_t bus, uint16_t device);
            void selectDrivers(driver::DriverManager* driver_manager);
            PCI_DeviceDescriptor getDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function);
    };
}

#endif