#include "hardware_communication/kpci.hpp"
#include "basic/kiostream.hpp"

using namespace hardware_communication;

/// @brief Constructs a new PCI_DeviceDescriptor object.
PCI_DeviceDescriptor::PCI_DeviceDescriptor(){}

/// @brief Destroys the PCI_DeviceDescriptor object.
PCI_DeviceDescriptor::~PCI_DeviceDescriptor(){}



/// @brief Constructs a new PCI_Controller, initializing the command and data ports.
PCI_Controller::PCI_Controller() : dataPort(0xCFC), commandPort(0xCF8) {}

/// @brief Destroys the PCI_Controller object.
PCI_Controller::~PCI_Controller(){}

/// @brief Helper to get the full 32-bit address for the command port
uint32_t PCI_Controller::getAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    // Casts to uint32_t for bitwise operations
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;

    // Create configuration address
    // Formula: (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (registerOffset & 0xFC)
    return 0x80000000 
         | (lbus << 16)
         | (ldevice << 11)
         | (lfunction << 8)
         | (registerOffset & 0xFC); // Mask to get 32-bit aligned offset
}


/// @brief Reads a 32-bit DWORD from the PCI config space.
uint32_t PCI_Controller::read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    // Get the 32-bit aligned address.
    // Note: registerOffset & 0xFC is handled by getAddress()
    uint32_t address = getAddress(bus, device, function, registerOffset);
    commandPort.write(address);
    
    // Read the full 32-bit DWORD from the data port
    return dataPort.read();
}

/// @brief Reads a 16-bit WORD from the PCI config space.
uint16_t PCI_Controller::read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    // Read the full 32-bit DWORD that contains the desired word
    uint32_t dword = read32(bus, device, function, registerOffset);
    
    // Calculate the shift (0 or 16 bits)
    // (registerOffset % 4) will be 0 or 2 for a 16-bit aligned read
    uint32_t shift = (8 * (registerOffset % 4));
    
    // Shift the desired word to the low bits and cast to uint16_t
    return (uint16_t)(dword >> shift);
}

/// @brief Reads an 8-bit BYTE from the PCI config space.
uint8_t PCI_Controller::read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    // Read the full 32-bit DWORD that contains the desired byte
    uint32_t dword = read32(bus, device, function, registerOffset);

    // Calculate the shift (0, 8, 16, or 24 bits)
    uint32_t shift = (8 * (registerOffset % 4));
    
    // Shift the desired byte to the low bits and cast to uint8_t
    return (uint8_t)(dword >> shift);
}

/// @brief Writes a 32-bit DWORD to the PCI config space.
void PCI_Controller::write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint32_t value)
{
    // This is a direct write, assuming the offset is 32-bit aligned.
    uint32_t address = getAddress(bus, device, function, registerOffset);
    commandPort.write(address);
    dataPort.write(value);
}

/// @brief (Safely) Writes a 16-bit WORD to the PCI config space.
/// @details Performs a read-modify-write to avoid corrupting adjacent bytes.
void PCI_Controller::write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint16_t value)
{
    // This must be a Read-Modify-Write to avoid corrupting adjacent bytes.
    
    // Read the current 32-bit DWORD
    uint32_t dword = read32(bus, device, function, registerOffset);
    
    // Modify
    uint32_t shift = (8 * (registerOffset % 4)); // 0 or 16
    uint32_t mask = 0xFFFF << shift; // Mask for the 16 bits
    
    dword &= ~mask; // Clear the old 16 bits
    dword |= ((uint32_t)value << shift); // Set the new 16 bits
    
    // Write
    write32(bus, device, function, registerOffset, dword);
}

/// @brief (Safely) Writes an 8-bit BYTE to the PCI config space.
/// @details Performs a read-modify-write to avoid corrupting adjacent bytes.
void PCI_Controller::write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint8_t value)
{
    // Read
    uint32_t dword = read32(bus, device, function, registerOffset);
    
    // Modify
    uint32_t shift = (8 * (registerOffset % 4)); // 0, 8, 16, or 24
    uint32_t mask = 0xFF << shift; // Mask for the 8 bits
    
    dword &= ~mask; // Clear the old 8 bits
    dword |= ((uint32_t)value << shift); // Set the new 8 bits
    
    // Write
    write32(bus, device, function, registerOffset, dword);
}


/// @brief Checks if a device is a multi-function device.
bool PCI_Controller::deviceHasFunctions(uint8_t bus, uint8_t device)
{
    // Read Header Type (offset 0x0E), check bit 7 (Multi-function bit)
    // 0x80 = (1 << 7)
    return (read8(bus, device, 0, 0x0E) & 0x80);
}

/// @brief Fills a PCI_DeviceDescriptor struct for a given device function.
PCI_DeviceDescriptor PCI_Controller::getDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function)
{
    PCI_DeviceDescriptor result;
    result.bus =        bus;
    result.device =     device;
    result.function =   function;

    result.vendorId =   read16(bus, device, function, 0x00);
    result.deviceId =   read16(bus, device, function, 0x02);

    result.classId =        read8(bus, device, function, 0x0B);
    result.subclassId =     read8(bus, device, function, 0x0A);
    result.interfaceId =    read8(bus, device, function, 0x09);
    result.revision =       read8(bus, device, function, 0x08);

    result.headerType =     read8(bus, device, function, 0x0E);
    result.interrupt =      read8(bus, device, function, 0x3C);

    // Base Address Registers (BARs)
    result.bar0 = read32(bus, device, function, 0x10);
    result.bar1 = read32(bus, device, function, 0x14);
    result.bar2 = read32(bus, device, function, 0x18);
    result.bar3 = read32(bus, device, function, 0x1C);
    result.bar4 = read32(bus, device, function, 0x20);
    result.bar5 = read32(bus, device, function, 0x24);

    return result;
}

/// @brief Scans a single PCI bus for devices and recursively scans bridges.
void PCI_Controller::scanBus(uint8_t bus, driver::DriverManager* driver_manager)
{
    for(int device=0; device<32; device++)
    {
        // Use the new signature with uint8_t
        int numFunctions = deviceHasFunctions(bus, (uint8_t)device) ? 8 : 1;
        
        for(int function=0; function<numFunctions; function++)
        {
            // Use the new signature with uint8_t
            PCI_DeviceDescriptor dev = getDeviceDescriptor(bus, (uint8_t)device, (uint8_t)function);

            // Check if device exists
            if(dev.vendorId == 0x0000 || dev.vendorId == 0xFFFF) 
                continue;
            
            basic::printf("PCI BUS %#3x, DEVICE %#3x, FUNCTION %#3x ", bus, device, function);
            basic::printf("= VENDOR %#5hx, DEVICE %#5hx (CLASS %#3x, SUB %#3x)\n", 
                          dev.vendorId, dev.deviceId, dev.classId, dev.subclassId);
            
            // driver_manager->registerDevice(dev); // TODO: Uncomment when ready

            // Check if it's a PCI-to-PCI bridge
            // Class 0x06 = Bridge, Subclass 0x04 = PCI-to-PCI Bridge
            // We can also check (dev.headerType & 0x7F) == 0x01
            if(dev.classId == 0x06 && dev.subclassId == 0x04)
            {
                // It's a bridge. Read its secondary bus number. (Register 0x19)
                uint8_t secondaryBus = read8(bus, (uint8_t)device, (uint8_t)function, 0x19);
                
                // Recursively scan the bus on the other side
                if (secondaryBus != bus) { 
                    scanBus(secondaryBus, driver_manager); 
                }
            }
        }
    }
}

/// @brief Begins the PCI bus scan from Bus 0.
void PCI_Controller::selectDrivers(driver::DriverManager* driver_manager)
{
    // Start the recursive scan from Bus 0
    scanBus(0, driver_manager); 
}

