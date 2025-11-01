#include "driver/kserial.hpp"
#include "basic/kiostream.hpp"

using namespace driver;
using namespace hardware_communication;

SerialEventHandler::SerialEventHandler(){}
SerialEventHandler::~SerialEventHandler(){}

SerialDriver::SerialDriver(InterruptManager* interrupt_manager, SerialEventHandler* eventHandler)
:   InterruptHandler(0x24, interrupt_manager), // 0x24 = IRQ 4
    dataPort(0x3F8),
    intEnablePort(0x3F9),      // Data Port + 1
    modemControlPort(0x3BC),   // Data Port + 4
    lineStatusPort(0x3FD)      // Data Port + 5
{
    this->eventHandler = eventHandler;
}

SerialDriver::~SerialDriver(){}

uint32_t SerialDriver::handleInterrupt(uint32_t esp)
{
    if (eventHandler == 0) {
        dataPort.read();
        return esp;
    }
    if (lineStatusPort.read() & 0x01){
        char data = dataPort.read();
        eventHandler->onDataReceived(data);
    }
    
    return esp;
}

/// @brief Activates the serial driver by enabling its interrupts.
void SerialDriver::activate()
{
    intEnablePort.write(0x01); 
    modemControlPort.write(0x08);
    
    basic::printf("Serial Driver activated!\n");
}

int SerialDriver::reset()
{
    activate();
    return 0;
}

void SerialDriver::deactivate()
{
    // Disable all serial port interrupts
    intEnablePort.write(0x00);
    // Disconnect from PIC
    modemControlPort.write(0x00);
    basic::printf("Serial Driver deactivated.\n");
}

