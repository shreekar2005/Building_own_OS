#include "hardware_communication/kpci.hpp"
using namespace hardware_communication;

PCI_Controller::PCI_Controller()
:dataPort(0xCFC),
 commandPort(0xCF8)
 {}

PCI_Controller::~PCI_Controller(){}


uint16_t PCI_Controller::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)device;
    uint32_t lfunc = (uint32_t)function;
    uint16_t tmp = 0;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (registerOffset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    commandPort.write(address);
    // Read in the data
    // (registerOffset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((dataPort.read() >> ((registerOffset & 2) * 8)) & 0xFFFF);
    return tmp;
}

void PCI_Controller::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOffset, uint32_t value)
{

}