#ifndef _OSOS_DRIVER_KMOUSE_H
#define _OSOS_DRIVER_KMOUSE_H

#include <cstdint>
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kport.hpp"
#include "driver/kdriver.hpp"

#define KMOUSE_COLOR_BLACK         0x0
#define KMOUSE_COLOR_BLUE          0x1
#define KMOUSE_COLOR_GREEN         0x2
#define KMOUSE_COLOR_CYAN          0x3
#define KMOUSE_COLOR_RED           0x4
#define KMOUSE_COLOR_MAGENTA       0x5
#define KMOUSE_COLOR_BROWN         0x6
#define KMOUSE_COLOR_LIGHT_GREY    0x7
#define KMOUSE_COLOR_DARK_GREY     0x8
#define KMOUSE_COLOR_LIGHT_BLUE    0x9 
#define KMOUSE_COLOR_LIGHT_GREEN   0xA
#define KMOUSE_COLOR_LIGHT_CYAN    0xB
#define KMOUSE_COLOR_LIGHT_RED     0xC
#define KMOUSE_COLOR_LIGHT_MAGENTA 0xD
#define KMOUSE_COLOR_YELLOW        0xE
#define KMOUSE_COLOR_WHITE         0xF
#define MOUSE_POINTER_COLOR KMOUSE_COLOR_GREEN //Here actually you will choose which color should your mouse pointer have

namespace driver
{
/// @brief Base class (interface) for handling mouse events (movement and button clicks).
class MouseEventHandler{
    public:
        MouseEventHandler();
        virtual ~MouseEventHandler();
        virtual void onMouseDown(uint8_t button)=0;
        virtual void onMouseUp(uint8_t button)=0;
        virtual void onMouseMove(int8_t delta_x, int8_t delta_y)=0;
};

/// @brief Custom Driver for the PS/2 mouse, handling 3-byte packets and managing mouse state.
class MouseDriver : public hardware_communication::InterruptHandler, public driver::Driver{
        hardware_communication::Port8Bit dataPort;
        hardware_communication::Port8Bit commandPort;
        int8_t buffer[3];
        uint8_t offset;
        uint8_t buttons;
        MouseEventHandler *mouseEventHandler;
        
    public :
        static uint16_t old_char_under_mouse_pointer;
        static int8_t __mouse_x_, __mouse_y_;
        MouseDriver(hardware_communication::InterruptManager* interrupt_manager, MouseEventHandler* mouseEventHandler);
        ~MouseDriver();

        /// @brief Calculates the video memory value for the mouse cursor block.
        /// @param current_char The original character attributes and ASCII value at the cursor position.
        /// @param mouse_pointer_color The desired background color for the mouse pointer.
        /// @return The new 16-bit video memory value representing the character with the new background color.
        static uint16_t mouse_block_video_mem_value(uint16_t current_char, uint8_t mouse_pointer_color);

        /// @brief Handles the mouse interrupt (IRQ 12).
        /// @param esp The stack pointer from the interrupt context.
        /// @return The stack pointer.
        uint32_t handleInterrupt(uint32_t esp) override;
        
        /// @brief Activates the mouse driver.
        void activate() override;

        /// @brief Resets the mouse. (Stub)
        /// @return Always returns 0.
        int reset() override;

        /// @brief Deactivates the mouse driver. (Stub)
        void deactivate() override;
};
}

#endif