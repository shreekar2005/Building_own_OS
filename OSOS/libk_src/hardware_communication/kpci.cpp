#include "hardware_communication/kpci.hpp"
#include "basic/kiostream.hpp"

using namespace hardware_communication;

/// @brief Constructs a new PCI_DeviceDescriptor object.
PCI_DeviceDescriptor::PCI_DeviceDescriptor()
{
    for(int i=0; i<6; i++) {
        bar[i] = 0;
        portBase[i] = 0;
    }
}

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

    result.revision =       read8(bus, device, function, 0x08);
    result.interfaceId =    read8(bus, device, function, 0x09);
    result.subclassId =     read8(bus, device, function, 0x0A);
    result.classId =        read8(bus, device, function, 0x0B);

    result.headerType = read8(bus, device, function, 0x0E);
    
    // Base Address Registers (BARs)
    for(int i = 0; i < 6; i++){
        result.bar[i] = read32(bus, device, function, 0x10 + (i * 4));
    }
    
    result.interrupt =  read8(bus, device, function, 0x3C);

    return result;
}

/// @brief Scans a single PCI bus for devices and recursively scans bridges and set drivers.
void PCI_Controller::recursiveSelDriver(uint8_t bus, driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager)
{
    for(uint8_t device=0; device<32; device++)
    {
        // Use the new signature with uint8_t
        uint8_t numFunctions = deviceHasFunctions(bus, device) ? 8 : 1;
        
        for(uint8_t function=0; function<numFunctions; function++)
        {
            // Use the new signature with uint8_t
            PCI_DeviceDescriptor dev = getDeviceDescriptor(bus, device, function);

            // Check if device exists
            if(dev.vendorId == 0x0000 || dev.vendorId == 0xFFFF) continue;
            
            basic::printf("PCI Bus %#3x, Device %#3x, Function %#3x ", bus, device, function);
            basic::printf("=> Vendor %#5hx, Device %#5hx (Class %#3x, Sub %#3x)\n", dev.vendorId, dev.deviceId, dev.classId, dev.subclassId);

            for(uint16_t barNum = 0; barNum < 6; barNum++){
                BaseAddressRegister bar = getBaseAddressRegister(bus, device, function, barNum);
                // If we find a valid I/O BAR, save it as the portBase
                if(bar.address && (bar.type == inputOutput)) dev.portBase[barNum] = (uint32_t)bar.address;
            }
            driver::Driver* driver = getDriver(dev, interrupt_manager);
            if(driver != 0) driver_manager->addDriver(driver);
            
            // Check if it's a PCI-to-PCI bridge
            // Class 0x06 = Bridge, Subclass 0x04 = PCI-to-PCI Bridge
            if(dev.classId == 0x06 && dev.subclassId == 0x04)
            {
                // It's a bridge. Read its secondary bus number. (Register 0x19)
                uint8_t secondaryBus = read8(bus, device, function, 0x19);
                
                // Recursively scan the bus on the other side
                if (secondaryBus != bus) recursiveSelDriver(secondaryBus, driver_manager, interrupt_manager);
            }
        }
    }
}

/// @brief Begins the PCI bus scan from Bus 0 and set drivers for devices. also recurse for bridges
void PCI_Controller::selectDrivers(driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager)
{
    recursiveSelDriver(0, driver_manager, interrupt_manager); 
}

/// @brief Scans a single PCI bus for devices and recursively scans bridges.
void PCI_Controller::scanBus(uint8_t bus)
{
    for(uint8_t device=0; device<32; device++)
    {
        // Use the new signature with uint8_t
        uint8_t numFunctions = deviceHasFunctions(bus, (uint8_t)device) ? 8 : 1;
        
        for(uint8_t function=0; function<numFunctions; function++)
        {
            // Use the new signature with uint8_t
            PCI_DeviceDescriptor dev = getDeviceDescriptor(bus, device, function);

            // Check if device exists
            if(dev.vendorId == 0x0000 || dev.vendorId == 0xFFFF) continue;
            
            basic::printf("PCI Bus %#3x, Device %#3x, Function %#3x ", bus, device, function);
            basic::printf("=> Vendor %#5hx, Device %#5hx (Class %#3x, Sub %#3x)\n", dev.vendorId, dev.deviceId, dev.classId, dev.subclassId);

            switch(dev.vendorId)
            {
                case 0x1022: // AMD
                    switch(dev.deviceId)
                    {
                        case 0x2000:
                            basic::printf("AMD am79c973 ");
                            break;
                    }
                    break;

                case 0x8086: // Intel
                    switch(dev.deviceId)
                    {
                        case 0x1237: // Chipset: 440FX - 82441FX PMC [Natoma]
                            basic::printf("Intel 440FX Host Bridge "); 
                            break;
                            
                        case 0x7000: // Chipset: 82371SB PIIX3 ISA [Natoma/Triton II]
                            basic::printf("Intel PIIX3 ISA Bridge "); 
                            break;
                            
                        case 0x7010: // Disk Controller: 82371SB PIIX3 IDE [Natoma/Triton II]
                            basic::printf("Intel PIIX3 IDE Controller "); 
                            break;
                            
                        case 0x7113: // Power Management: 82371AB/EB/MB PIIX4 ACPI
                            basic::printf("Intel PIIX4 ACPI "); 
                            break;
                            
                        case 0x100E: // Network: 82540EM Gigabit Ethernet Controller (E1000)
                            basic::printf("Intel E1000 Ethernet "); 
                            // Note: This is a very common network card in QEMU.
                            break;
                    }
                    break;
                    
                case 0x1234: // QEMU / Bochs / VirtualBox specific vendor
                    switch(dev.deviceId)
                    {
                        case 0x1111: // QEMU Standard VGA
                            basic::printf("QEMU VGA Graphics ");
                            break;
                    }
                    break;
            }

            basic::printf("| ");

            // Check Generic Device Classes
            // If we didn't identify the specific device above, try to guess based on class.
            switch(dev.classId)
            {
                case 0x03: // Graphics Display Controller
                    switch(dev.subclassId)
                    {
                        case 0x00: // VGA Compatible
                            basic::printf("Generic VGA ");
                            break;
                    }
                    break;
                    
                case 0x02: // Network Controller
                    basic::printf("Generic Network Card ");
                    break;
            }

            basic::printf("|\n");

            // Check if it's a PCI-to-PCI bridge
            // Class 0x06 = Bridge, Subclass 0x04 = PCI-to-PCI Bridge
            if(dev.classId == 0x06 && dev.subclassId == 0x04)
            {
                // It's a bridge. Read its secondary bus number. (Register 0x19)
                uint8_t secondaryBus = read8(bus, device, function, 0x19);
            
                // Recursively scan the bus on the other side
                if (secondaryBus != bus) scanBus(secondaryBus);
            }
        }
    }
}

BaseAddressRegister PCI_Controller::getBaseAddressRegister(uint8_t bus, uint8_t device, uint8_t function, uint16_t bar)
{
    BaseAddressRegister result;

    // prevents reading garbage on some devices
    uint32_t headertype = read8(bus, device, function, 0x0E) & 0x7F;
    int maxBARs = 6 - (4 * headertype);
    if(bar >= maxBARs) return result;

    // Read the Raw Value
    // We read it again here to ensure we are decoding the fresh value from the register
    uint32_t bar_value = read32(bus, device, function, 0x10 + (4 * bar));
    
    // Decode Type (Bit 0)
    result.type = (bar_value & 0x1) ? inputOutput : memoryMapping;

    if(result.type == memoryMapping)
    {
        // Memory Mapped: Mask last 4 bits (0xF)
        result.address = (uint8_t*)(bar_value & ~0xF);
        
        // Prefetchable is Bit 3 in Memory Mapped BARs
        result.prefetchable = ((bar_value & 0x8) != 0);
    }
    else // InputOutput
    {
        // I/O Mapped: Mask last 2 bits (0x3)
        result.address = (uint8_t*)(bar_value & ~0x3);
        
        // I/O is never prefetchable
        result.prefetchable = false;
    }

    return result;
}

driver::Driver* PCI_Controller::getDriver(PCI_DeviceDescriptor dev, hardware_communication::InterruptManager* interrupt_manager)
{
    (void) interrupt_manager; // TASK: try to remove this faaltu line without any warning :)

    driver::Driver* driver = 0;
    switch(dev.vendorId)
    {
        case 0x1022: // AMD
            switch(dev.deviceId)
            {
                case 0x2000: // am79c973 (PCnet-FAST III)
                    basic::printf("AMD am79c973 ");
                    // (example) Instantiate AMD driver here...
                    // driver = new amd_am79c973(&dev, interrupt_manager);
                    break;
            }
            break;

        case 0x8086: // Intel
            switch(dev.deviceId)
            {
                case 0x1237: // Chipset: 440FX - 82441FX PMC [Natoma]
                    basic::printf("Intel 440FX Host Bridge "); 
                    break;
                    
                case 0x7000: // Chipset: 82371SB PIIX3 ISA [Natoma/Triton II]
                    basic::printf("Intel PIIX3 ISA Bridge "); 
                    break;
                    
                case 0x7010: // Disk Controller: 82371SB PIIX3 IDE [Natoma/Triton II]
                    basic::printf("Intel PIIX3 IDE Controller "); 
                    break;
                    
                case 0x7113: // Power Management: 82371AB/EB/MB PIIX4 ACPI
                    basic::printf("Intel PIIX4 ACPI "); 
                    break;
                    
                case 0x100E: // Network: 82540EM Gigabit Ethernet Controller (E1000)
                    basic::printf("Intel E1000 Ethernet "); 
                    // Note: This is a very common network card in QEMU.
                    break;
            }
            break;
            
        case 0x1234: // QEMU / Bochs / VirtualBox specific vendor
            switch(dev.deviceId)
            {
                case 0x1111: // QEMU Standard VGA
                    basic::printf("QEMU VGA Graphics ");
                    break;
            }
            break;
    }
    basic::printf("| ");
    // Check Generic Device Classes
    // If we didn't identify the specific device above, try to guess based on class.
    switch(dev.classId)
    {
        case 0x03: // Graphics Display Controller
            switch(dev.subclassId)
            {
                case 0x00: // VGA Compatible
                    basic::printf("Generic VGA ");
                    break;
            }
            break;
            
        case 0x02: // Network Controller
            basic::printf("Generic Network Card ");
            break;
    }

    basic::printf("|\n---\n");
    return driver;
}