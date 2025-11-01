#ifndef _OSOS_DRIVER_KKEYBOARD_H
#define _OSOS_DRIVER_KKEYBOARD_H

#include <cstdint>
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"
#include "driver/kdriver.hpp"

namespace driver
{
/// @brief Base class (interface) for handling keyboard events (key presses and releases).
class KeyboardEventHandler{
    public:
        KeyboardEventHandler();
        virtual ~KeyboardEventHandler();
        virtual void onKeyDown(char ascii)=0;
        virtual void onKeyUp(char ascii)=0;
};

/// @brief Custom Driver for the PS/2 keyboard, handling scancodes and managing keyboard state (Shift, Caps Lock).
class KeyboardDriver : public hardware_communication::InterruptHandler, public driver::Driver{
        hardware_communication::Port8Bit dataPort;
        hardware_communication::Port8Bit commandPort; 
        bool shift_pressed;
        bool caps_on;
        bool waiting_for_led_ack;
        uint8_t led_byte_to_send;
        KeyboardEventHandler* keyboardEventHandler;

    public :
        /// @brief Constructs a new KeyboardDriver object.
        /// @param interrupt_manager Pointer to the interrupt manager.
        /// @param keyboardEventHandler Pointer to the event handler that will process key events.
        KeyboardDriver(hardware_communication::InterruptManager* interrupt_manager, KeyboardEventHandler* keyboardEventHandler);

        ~KeyboardDriver();

        /// @brief Handles the keyboard interrupt (IRQ 1).
        /// @param esp The stack pointer from the interrupt context.
        /// @return The stack pointer.
        uint32_t handleInterrupt(uint32_t esp) override;

        /// @brief Activates the keyboard driver.
        void activate() override;

        /// @brief Resets the keyboard. (Stub)
        /// @return Always returns 0.
        int reset() override;
        
        /// @brief Deactivates the keyboard driver. (Stub)
        void deactivate() override;
};
}

#endif