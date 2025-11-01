#ifndef _OSOS_DRIVER_KSERIAL_H
#define _OSOS_DRIVER_KSERIAL_H

#include <cstdint>

#include "driver/kdriver.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"

namespace driver 
{
/// @brief Interface for handling serial port events
class SerialEventHandler{
    public:
        SerialEventHandler();
        virtual ~SerialEventHandler();
        virtual void onDataReceived(char data)=0;
};

/// @brief Driver for the COM1 serial port (0x3F8)
class SerialDriver : public hardware_communication::InterruptHandler, public driver::Driver{
private:
    hardware_communication::Port8Bit dataPort;
    hardware_communication::Port8Bit intEnablePort;
    hardware_communication::Port8Bit modemControlPort;
    hardware_communication::Port8Bit lineStatusPort;
    SerialEventHandler* eventHandler;

public:
    SerialDriver(hardware_communication::InterruptManager* interrupt_manager, SerialEventHandler* eventHandler);
    ~SerialDriver();

    /// @brief Handles the serial port interrupt (IRQ 4).
    /// @param esp The stack pointer from the interrupt context.
    /// @return The stack pointer.
    /// @details Reads a character from the data port (which clears the interrupt) and forwards it to the event handler.
    uint32_t handleInterrupt(uint32_t esp) override;

    /// @brief Activates the serial driver by enabling its interrupts.
    void activate() override;

    /// @brief Resets the serial driver. (Stub)
    /// @return Always returns 0.
    int reset() override;

    /// @brief Deactivates the serial driver by disabling its interrupts.
    void deactivate() override;
};
}

#endif

