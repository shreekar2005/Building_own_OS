#ifndef _OSOS_HARDWARECOMMUNICATION_KPCI_H
#define _OSOS_HARDWARECOMMUNICATION_KPCI_H

#include "essential/ktypes.hpp"
#include "hardware_communication/kport.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "driver/kdriver.hpp"

namespace hardware_communication
{
    /// @brief Defines the type of a Base Address Register (BAR).
    enum BaseAddressRegisterType{
        memoryMapping = 0, // device maps its registers into system memory (RAM address space)
        inputOutput = 1    // device uses I/O ports.
    };
    
    /// @brief Represents a parsed Base Address Register (BAR) from a PCI device.
    class BaseAddressRegister{
        public:
            bool prefetchable; // true if reading from this memory has no side effects
            uint8_t* address; // physical address (for memory mapping) or port number (for I/O) cast to a pointer
            uint32_t size; // size of the memory region (currently unused/uncalculated)
            BaseAddressRegisterType type; // type of mapping (Memory or I/O)
    };

    /// @brief A descriptor containing all essential information about a specific PCI device function.
    class PCI_DeviceDescriptor{
        public:
            uint8_t bus;        // bus number (0-255).
            uint8_t device;     // device number on the bus (0-31).
            uint8_t function;   // function number within the device (0-7).

            uint16_t vendorId;  // identifies the manufacturer (e.g., 0x8086 for Intel).
            uint16_t deviceId;  // identifies the specific device model.

            uint8_t revision;   // device revision ID.
            uint8_t interfaceId;// programming Interface Byte (Prog IF), identifying specific register layouts.
            uint8_t subclassId; // specific type of device (e.g., IDE Controller).
            uint8_t classId;    // general type of device (e.g., Mass Storage Controller).
            
            uint8_t headerType; // defines the layout of the rest of the header (Standard vs Bridge).
            
            uint32_t bar[6];    // raw values of the 6 Base Address Registers.
            
            uint8_t interrupt;  // interrupt line (IRQ) used by this device.

            uint32_t portBase[6]; // stores the decoded I/O port base address if a BAR is type InputOutput.

            PCI_DeviceDescriptor();
            ~PCI_DeviceDescriptor();
    };

    /// @brief Manages communication with the PCI bus via the x86 Configuration Space mechanism.
    class PCI_Controller{
        private:
            Port32Bit dataPort;     // Port 0xCFC: 32-bit data port for reading/writing config data
            Port32Bit commandPort;  // Port 0xCF8: 32-bit command/address port

            /// @brief Helper to construct the 32-bit CONFIG_ADDRESS.
            /// @param bus bus number (0-255).
            /// @param device device number (0-31).
            /// @param function function number (0-7).
            /// @param registerOffset offset into the configuration space (must be 4-byte aligned).
            /// @return A 32-bit integer formatted for port 0xCF8.
            uint32_t getAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);

        public:
            PCI_Controller();
            ~PCI_Controller();

            /// @brief Reads a 32-bit double word from the PCI configuration space.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param registerOffset offset (will be aligned automatically).
            /// @return 32-bit value at the register.
            uint32_t read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);

            /// @brief Reads a 16-bit word from the PCI configuration space.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param registerOffset offset.
            /// @return 16-bit value.
            uint16_t read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);

            /// @brief Reads an 8-bit byte from the PCI configuration space.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param registerOffset offset.
            /// @return 8-bit value.
            uint8_t read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset);

            /// @brief Writes a 32-bit double word to the PCI configuration space.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param registerOffset offset.
            /// @param value 32-bit value to write.
            void write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint32_t value);

            /// @brief Safely writes a 16-bit word to the PCI configuration space.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param registerOffset offset.
            /// @param value 16-bit value to write.
            void write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint16_t value);

            /// @brief Safely writes an 8-bit byte to the PCI configuration space.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param registerOffset offset.
            /// @param value 8-bit value to write.
            void write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint8_t value);
            
            /// @brief Checks if a device is a multi-function device.
            /// @param bus bus number.
            /// @param device device number.
            /// @return True if the device has multiple functions (bit 7 of Header Type is set), false otherwise.
            bool deviceHasFunctions(uint8_t bus, uint8_t device);

            /// @brief Reads the configuration header to create a descriptor object.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @return A populated PCI_DeviceDescriptor object.
            PCI_DeviceDescriptor getDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function);

            /// @brief Recursively scans the bus for devices and bridges, assigning drivers where possible.
            /// @param bus current bus being scanned.
            /// @param driver_manager Pointer to the global driver manager.
            /// @param interrupt_manager Pointer to the global interrupt manager.
            void recursiveSelDriver(uint8_t bus, driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager);

            /// @brief Starts the driver selection process from Bus 0.
            /// @param driver_manager Pointer to the global driver manager.
            /// @param interrupt_manager Pointer to the global interrupt manager.
            void selectDrivers(driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager);

            /// @brief Factory method that instantiates the correct driver for a given device.
            /// @param dev device descriptor containing VendorID and DeviceID.
            /// @param interrupt_manager interrupt manager to pass to the driver.
            /// @return A pointer to the new driver instance, or 0 if no driver is found.
            driver::Driver* getDriver(PCI_DeviceDescriptor dev, hardware_communication::InterruptManager* interrupt_manager);

            /// @brief Decodes a raw BAR value into a BaseAddressRegister object.
            /// @param bus bus number.
            /// @param device device number.
            /// @param function function number.
            /// @param bar index of the BAR to read (0-5).
            /// @return A BaseAddressRegister struct with the decoded address and type.
            BaseAddressRegister getBaseAddressRegister(uint8_t bus, uint8_t device, uint8_t function, uint16_t bar);

            /// @brief Scans a bus just for listing devices (does not instantiate drivers).
            /// @param bus bus number to scan.
            void scanBus(uint8_t bus);
    };
}

#endif