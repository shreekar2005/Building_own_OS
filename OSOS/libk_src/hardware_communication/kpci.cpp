#include "hardware_communication/kpci.hpp"
#include "basic/kiostream.hpp"

using namespace hardware_communication;

PCI_DeviceDescriptor::PCI_DeviceDescriptor()
{
    for(int i=0; i<6; i++) {
        bar[i] = 0;
        portBase[i] = 0;
    }
}

PCI_DeviceDescriptor::~PCI_DeviceDescriptor(){}

PCI_Controller::PCI_Controller() : dataPort(0xCFC), commandPort(0xCF8) {}

PCI_Controller::~PCI_Controller(){}

uint32_t PCI_Controller::getAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunction = (uint32_t)function;

    return 0x80000000 
         | (lbus << 16)
         | (ldevice << 11)
         | (lfunction << 8)
         | (registerOffset & 0xFC);
}

uint32_t PCI_Controller::read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    uint32_t address = getAddress(bus, device, function, registerOffset);
    commandPort.write(address);
    return dataPort.read();
}

uint16_t PCI_Controller::read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    uint32_t dword = read32(bus, device, function, registerOffset);
    uint32_t shift = (8 * (registerOffset % 4));
    return (uint16_t)(dword >> shift);
}

uint8_t PCI_Controller::read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset)
{
    uint32_t dword = read32(bus, device, function, registerOffset);
    uint32_t shift = (8 * (registerOffset % 4));
    return (uint8_t)(dword >> shift);
}

void PCI_Controller::write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint32_t value)
{
    uint32_t address = getAddress(bus, device, function, registerOffset);
    commandPort.write(address);
    dataPort.write(value);
}

void PCI_Controller::write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint16_t value)
{
    uint32_t dword = read32(bus, device, function, registerOffset);
    
    uint32_t shift = (8 * (registerOffset % 4));
    uint32_t mask = 0xFFFF << shift;
    
    dword &= ~mask;
    dword |= ((uint32_t)value << shift);
    
    write32(bus, device, function, registerOffset, dword);
}

void PCI_Controller::write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerOffset, uint8_t value)
{
    uint32_t dword = read32(bus, device, function, registerOffset);
    
    uint32_t shift = (8 * (registerOffset % 4));
    uint32_t mask = 0xFF << shift;
    
    dword &= ~mask;
    dword |= ((uint32_t)value << shift);
    
    write32(bus, device, function, registerOffset, dword);
}

bool PCI_Controller::deviceHasFunctions(uint8_t bus, uint8_t device)
{
    return (read8(bus, device, 0, 0x0E) & 0x80);
}

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
    
    for(int i = 0; i < 6; i++){
        result.bar[i] = read32(bus, device, function, 0x10 + (i * 4));
    }
    
    result.interrupt =  read8(bus, device, function, 0x3C);

    return result;
}

void PCI_Controller::recursiveSelDriver(uint8_t bus, driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager)
{
    for(uint8_t device=0; device<32; device++)
    {
        uint8_t numFunctions = deviceHasFunctions(bus, device) ? 8 : 1;
        
        for(uint8_t function=0; function<numFunctions; function++)
        {
            PCI_DeviceDescriptor dev = getDeviceDescriptor(bus, device, function);

            if(dev.vendorId == 0x0000 || dev.vendorId == 0xFFFF) continue;
            
            basic::printf("PCI Bus %#3x, Device %#3x, Function %#3x ", bus, device, function);
            basic::printf("=> Vendor %#5hx, Device %#5hx (Class %#3x, Sub %#3x)\n", dev.vendorId, dev.deviceId, dev.classId, dev.subclassId);

            for(uint16_t barNum = 0; barNum < 6; barNum++){
                BaseAddressRegister bar = getBaseAddressRegister(bus, device, function, barNum);
                if(bar.address && (bar.type == inputOutput)) dev.portBase[barNum] = (uint32_t)bar.address;
            }
            driver::Driver* driver = getDriver(dev, interrupt_manager);
            if(driver != 0) driver_manager->addDriver(driver);
            
            if(dev.classId == 0x06 && dev.subclassId == 0x04)
            {
                uint8_t secondaryBus = read8(bus, device, function, 0x19);
                if (secondaryBus != bus) recursiveSelDriver(secondaryBus, driver_manager, interrupt_manager);
            }
        }
    }
}

void PCI_Controller::selectDrivers(driver::DriverManager* driver_manager, hardware_communication::InterruptManager* interrupt_manager)
{
    recursiveSelDriver(0, driver_manager, interrupt_manager); 
}

void PCI_Controller::scanBus(uint8_t bus)
{
    for(uint8_t device=0; device<32; device++)
    {
        uint8_t numFunctions = deviceHasFunctions(bus, (uint8_t)device) ? 8 : 1;
        
        for(uint8_t function=0; function<numFunctions; function++)
        {
            PCI_DeviceDescriptor dev = getDeviceDescriptor(bus, device, function);

            if(dev.vendorId == 0x0000 || dev.vendorId == 0xFFFF) continue;
            basic::printf("---\n");
            basic::printf("PCI Bus %#3x, Device %#3x, Function %#3x ", bus, device, function);
            basic::printf("=> Vendor %#5hx, Device %#5hx (Class %#3x, Sub %#3x)\n", dev.vendorId, dev.deviceId, dev.classId, dev.subclassId);

            switch(dev.vendorId)
            {
                case 0x1022: // AMD
                    switch(dev.deviceId)
                    {
                        case 0x2000: basic::printf("AMD am79c973 "); break;
                    }
                    break;

                case 0x8086: // Intel
                    switch(dev.deviceId)
                    {
                        case 0x1237: basic::printf("Intel 440FX Host Bridge "); break;
                        case 0x7000: basic::printf("Intel PIIX3 ISA Bridge "); break;
                        case 0x7010: basic::printf("Intel PIIX3 IDE Controller "); break;
                        case 0x7113: basic::printf("Intel PIIX4 ACPI "); break;
                        case 0x100E: basic::printf("Intel E1000 Ethernet "); break;
                    }
                    break;
                    
                case 0x1234: // QEMU / Bochs
                    switch(dev.deviceId)
                    {
                        case 0x1111: basic::printf("QEMU VGA Graphics "); break;
                    }
                    break;
            }

            basic::printf("| ");

            switch(dev.classId)
            {
                case 0x03:
                    if(dev.subclassId == 0x00) basic::printf("Generic VGA ");
                    break;
                case 0x02:
                    basic::printf("Generic Network Card ");
                    break;
            }

            basic::printf("|\n");

            if(dev.classId == 0x06 && dev.subclassId == 0x04)
            {
                uint8_t secondaryBus = read8(bus, device, function, 0x19);
                if (secondaryBus != bus) scanBus(secondaryBus);
            }
        }
    }
}

BaseAddressRegister PCI_Controller::getBaseAddressRegister(uint8_t bus, uint8_t device, uint8_t function, uint16_t bar)
{
    BaseAddressRegister result;

    uint32_t headertype = read8(bus, device, function, 0x0E) & 0x7F;
    int maxBARs = 6 - (4 * headertype);
    if(bar >= maxBARs) return result;

    uint32_t bar_value = read32(bus, device, function, 0x10 + (4 * bar));
    
    result.type = (bar_value & 0x1) ? inputOutput : memoryMapping;

    if(result.type == memoryMapping)
    {
        result.address = (uint8_t*)(bar_value & ~0xF);
        result.prefetchable = ((bar_value & 0x8) != 0);
    }
    else
    {
        result.address = (uint8_t*)(bar_value & ~0x3);
        result.prefetchable = false;
    }

    return result;
}

driver::Driver* PCI_Controller::getDriver(PCI_DeviceDescriptor dev, hardware_communication::InterruptManager* interrupt_manager)
{
    (void) interrupt_manager;

    driver::Driver* driver = 0;
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
                case 0x1237: basic::printf("Intel 440FX Host Bridge "); break;
                case 0x7000: basic::printf("Intel PIIX3 ISA Bridge "); break;
                case 0x7010: basic::printf("Intel PIIX3 IDE Controller "); break;
                case 0x7113: basic::printf("Intel PIIX4 ACPI "); break;
                case 0x100E: basic::printf("Intel E1000 Ethernet "); break;
            }
            break;
            
        case 0x1234:
            switch(dev.deviceId)
            {
                case 0x1111: basic::printf("QEMU VGA Graphics "); break;
            }
            break;
    }
    basic::printf("| ");
    
    switch(dev.classId)
    {
        case 0x03:
            if(dev.subclassId == 0x00) basic::printf("Generic VGA ");
            break;
        case 0x02:
            basic::printf("Generic Network Card ");
            break;
    }

    basic::printf("|\n---\n");
    return driver;
}