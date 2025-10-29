#include "hardware_communication/kpci.hpp"
using namespace hardware_communication;

PCI_DeviceDescriptor::PCI_DeviceDescriptor(){}
PCI_DeviceDescriptor::~PCI_DeviceDescriptor(){}

PCI_Controller::PCI_Controller() : dataPort(0xCFC), commandPort(0xCF8) {}
PCI_Controller::~PCI_Controller(){}


uint32_t PCI_Controller::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset)
{
    uint32_t address; // address is actually exact location = which bus, which device and which registerOffset
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)device;
    uint32_t lfunc = (uint32_t)function;

    // Create configuration address as per Figure 1
    address = 0x1<<31
            | (lbus << 16) 
            | (lslot << 11) 
            | (lfunc << 8) 
            | (registerOffset & 0xFC) ;

    // Write out the address
    commandPort.write(address);

    // Read in the data
    uint32_t result = dataPort.read() >> (8 * (registerOffset % 4));
    return result;
}

void PCI_Controller::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset, uint32_t value)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)device;
    uint32_t lfunc = (uint32_t)function;

    address = 0x1<<31
            | (lbus << 16) 
            | (lslot << 11) 
            | (lfunc << 8) 
            | (registerOffset & 0xFC) ;

    commandPort.write(address);
    dataPort.write(value);
}

bool PCI_Controller::deviceHasFunctions(uint16_t bus, uint16_t device)
{
    return read(bus, device, 0, 0x0E) & (1<<7);
}

void PCI_Controller::selectDrivers(driver::DriverManager* driver_manager)
{
    for(int bus=0; bus<8; bus++){
        for(int device=0; device<32; device++){
            int numFunctions = deviceHasFunctions(bus, device) ? 8 : 1;
            for(int function=0; function<numFunctions; function++){
                PCI_DeviceDescriptor dev = getDeviceDescriptor(bus, device, function);
                if(dev.vendorId == 0x0000 || dev.vendorId == 0xFFFF) continue;
                basic::printf("PCI BUS %#3x, DEVICE %#3x, FUNCTION %#3x ",bus&0xFF, device&0xFF, function&0xFF);
                basic::printf("= VENDOR %#5hx, DEVICE %#5hx\n", dev.vendorId, dev.deviceId);
            }
        }
    }
}

PCI_DeviceDescriptor PCI_Controller::getDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    PCI_DeviceDescriptor result;
    result.bus =        bus;
    result.device =     device;
    result.function =   function;

    result.vendorId =   read(bus, device, function, 0x00);
    result.deviceId =   read(bus, device, function, 0x02);

    result.classId =        read(bus, device, function, 0x0B);
    result.subclassId =     read(bus, device, function, 0x0A);
    result.interfaceId =    read(bus, device, function, 0x09);

    result.revision =   read(bus, device, function, 0x08);
    result.interrupt =  read(bus, device, function, 0x3C);
    return result;
}