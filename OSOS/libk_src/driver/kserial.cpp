#include "driver/kserial.hpp"
#include "basic/kiostream.hpp"

using namespace driver;
using namespace hardware_communication;


/// @brief Base class (interface) for handling serial port events.
/// @details Implement this class to create a listener that can be attached to the SerialDriver.
SerialEventHandler::SerialEventHandler(){}

/// @brief Destroys the SerialEventHandler object.
SerialEventHandler::~SerialEventHandler(){}


/// @brief Constructs a new SerialDriver object for COM1.
/// @param interrupt_manager Pointer to the interrupt manager.
/// @param eventHandler Pointer to the event handler that will process serial data.
SerialDriver::SerialDriver(InterruptManager* interrupt_manager, SerialEventHandler* eventHandler)
:   InterruptHandler(0x24, interrupt_manager), // 0x24 = IRQ 4
    dataPort(0x3F8),
    intEnablePort(0x3F9),      // Data Port + 1
    modemControlPort(0x3BC),   // Data Port + 4
    lineStatusPort(0x3FD)      // Data Port + 5
{
    this->eventHandler = eventHandler;
}

/// @brief Destroys the SerialDriver object.
SerialDriver::~SerialDriver(){}

/// @brief Handles the serial port interrupt (IRQ 4).
/// @param esp The stack pointer from the interrupt context.
/// @return The stack pointer.
/// @details Reads a character from the data port (which clears the interrupt) and forwards it to the event handler.
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

/// @brief Resets the serial driver. (Stub)
/// @return Always returns 0.
/// @details A real reset would involve more complex port manipulation. For now, we'll just re-activate.
int SerialDriver::reset()
{
    activate();
    return 0;
}

/// @brief Deactivates the serial driver by disabling its interrupts.
void SerialDriver::deactivate()
{
    // Disable all serial port interrupts
    intEnablePort.write(0x00);
    // Disconnect from PIC
    modemControlPort.write(0x00);
    basic::printf("Serial Driver deactivated.\n");
}

