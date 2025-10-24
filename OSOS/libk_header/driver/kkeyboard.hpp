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
                // for shift
                bool shift_pressed;
                // for caps
                bool caps_on;
                bool waiting_for_led_ack;
                uint8_t led_byte_to_send;

                KeyboardEventHandler* keyboardEventHandler;

            public :
                KeyboardDriver(hardware_communication::InterruptManager* interrupt_manager, KeyboardEventHandler* keyboardEventHandler);
                ~KeyboardDriver();
                uint32_t handleInterrupt(uint32_t esp) override; // function mainly defined in InterruptHandler class.
                void activate() override;
                int reset() override;
                void deactivate() override;
        };
    }

#endif