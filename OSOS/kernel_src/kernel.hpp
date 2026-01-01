#include <cstdint>

#include "basic/multiboot.h"
#include "basic/kiostream.hpp" 
#include "basic/kmemory.hpp"
#include "essential/kgdt.hpp"
#include "essential/kicxxabi.hpp"
#include "essential/kmultitasking.hpp"
#include "driver/kkeyboard.hpp"
#include "driver/kmouse.hpp"
#include "driver/kdriver.hpp"
#include "driver/kserial.hpp"
#include "hardware_communication/kinterrupt.hpp"
#include "hardware_communication/kpci.hpp"

/// @brief A simple shell/terminal manager for the kernel.
class KernelShell {
private:
    char m_line_buffer[256]; // Our internal line buffer (bcs after pressing enter we need full line buffer to process)
    int m_buffer_index; // Our current top position in the buffer
    hardware_communication::PCI_Controller pciController;

public:
    KernelShell()
    {
        m_buffer_index = 0;
        m_line_buffer[0] = '\0';
    }
    ~KernelShell() {}

    /// @brief The single, centralized function for handling all key presses.
    /// @param c The character received from any input driver (keyboard or serial driver).
    void handle_key(char c)
    {
        if (c == 0) return;

        switch(c)
        {
            // PS/2 Keyboard sends '\n' (from keyboard driver)
            // Serial sends '\r' (from serial driver)
            case '\r':
            case '\n':
                basic::printf("\n");
                m_line_buffer[m_buffer_index] = '\0';
                if (m_buffer_index > 0) {
                    process_command(m_line_buffer);
                }
                // Reset for next line
                m_buffer_index = 0;
                m_line_buffer[0] = '\0';
                basic::printf("\nOSOS> "); // Print a new prompt
                break;

            case 8: // Backspace '\b'
                if (m_buffer_index > 0){
                    m_buffer_index--;
                    m_line_buffer[m_buffer_index] = '\0';
                    basic::printf("\b \b"); // '\b' only move cursor back in serial terminal (e.g. my Ubuntu terminal), so i am printing ' ' once and then again '\b'
                }
                break;

            // Handle printable characters
            default:
                if (c >= ' ' && c <= '~' && m_buffer_index < 254)
                {
                    m_line_buffer[m_buffer_index++] = c;
                    m_line_buffer[m_buffer_index] = '\0';
                    basic::printf("%c", c);
                }
                break;
        }
    }

    /// @brief Simple command processor.
    /// @param command The null-terminated command string to process.
    void process_command(const char* command)
    {
        // Simple "strcmp"
        if (command[0] == 'h' && command[1] == 'e' && command[2] == 'l' && command[3] == 'p' && command[4] == '\0') {
            basic::printf("OSOS Kernel Shell. Type 'lspci' to list devices.");
        }
        else if (command[0] == 'l' && command[1] == 's' && command[2] == 'p' && command[3] == 'c' && command[4] == 'i' && command[5] == '\0') {
            basic::printf("Listing PCI devices...\n");
            pciController.scanBus(0);

        }
        else {
            basic::printf("Unknown command: '%s'", command);
        }
    }
};


/// @brief Custom KeyboardEventHandler implementation for kernel
class KeyboardEventHandler_for_kernel : public driver::KeyboardEventHandler
{
    private:
        KernelShell* m_shell; // Pointer to the central shell
    
    public:
        KeyboardEventHandler_for_kernel(KernelShell* shell) : m_shell(shell) {}
        ~KeyboardEventHandler_for_kernel(){}

        /// @brief Handles key down events by forwarding them to the shell.
        /// @param ascii The ASCII character of the key that was pressed.
        void onKeyDown(char ascii) override
        {
            if (ascii != 0 && m_shell != 0) {
                // kkeyboard.cpp already translates scancodes,
                // including '\b' for backspace and '\n' for enter.
                m_shell->handle_key(ascii);
            }
        }
        /// @brief Handles key up events. (Does nothing in this implementation).
        /// @param ascii The ASCII character of the key that was released.
        void onKeyUp(char ascii) override
        {
            (void) ascii;
        }
};

/// @brief Custom MouseEventHandler implementation for kernel.
class MouseEventHandler_for_kernel : public driver::MouseEventHandler{
    public:
        MouseEventHandler_for_kernel(){}
        ~MouseEventHandler_for_kernel(){}
        
        /// @brief Handles mouse button down events.
        /// @param button The button number that was pressed.
        void onMouseDown(uint8_t button) override
        {
            (void) button;
            basic::update_cursor(driver::MouseDriver::__mouse_x_, driver::MouseDriver::__mouse_y_);
            // printf("Mouse Down: %d\n", button);
        }

        /// @brief Handles mouse button up events.
        /// @param button The button number that was released.
        void onMouseUp(uint8_t button) override
        {
            (void) button; // just ignore
        }

        /// @brief Handles mouse movement events and updates the cursor position on the screen.
        /// @param delta_x The change in the x-coordinate.
        /// @param delta_y The change in the y-coordinate.
        void onMouseMove(int8_t delta_x, int8_t delta_y) override
        {
            static uint16_t* video_memory = (uint16_t*) 0xb8000;
            static float deltaTod=0.2;
            static float dx=0, dy=0;
            dx+=delta_x*deltaTod;
            dy+=delta_y*deltaTod;

            // Restore the character that was previously under the mouse pointer
            if (driver::MouseDriver::__mouse_x_ >= 0 && driver::MouseDriver::__mouse_y_ >= 0) {
                video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_] = driver::MouseDriver::old_char_under_mouse_pointer;
            }

            if(dx > 1) {
                driver::MouseDriver::__mouse_x_++;
                dx=0;
            }
            else if(dx < -1) {
                driver::MouseDriver::__mouse_x_--;
                dx=0;
            }
            
            if(dy > 1) {
                driver::MouseDriver::__mouse_y_++;
                dy=0;
            }
            else if(dy < -1) {
                driver::MouseDriver::__mouse_y_--;
                dy=0;
            }

            if(driver::MouseDriver::__mouse_x_ < 0) driver::MouseDriver::__mouse_x_ = 0;
            if(driver::MouseDriver::__mouse_x_ >= 80) driver::MouseDriver::__mouse_x_ = 79;
            if(driver::MouseDriver::__mouse_y_ < 0) driver::MouseDriver::__mouse_y_ = 0;
            if(driver::MouseDriver::__mouse_y_ >= 25) driver::MouseDriver::__mouse_y_ = 24;

            // Save the character at the new position and draw the pointer over it
            driver::MouseDriver::old_char_under_mouse_pointer = video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_];
            video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_] = driver::MouseDriver::mouse_block_video_mem_value(driver::MouseDriver::old_char_under_mouse_pointer, MOUSE_POINTER_COLOR);
        }
};

/// @brief Custom SerialEventHandler implementation for kernel
class SerialEventHandler_for_kernel : public driver::SerialEventHandler{
    private:
        KernelShell* m_shell; // Pointer to the central shell

    public:
        SerialEventHandler_for_kernel(KernelShell* shell) : m_shell(shell) {}
        ~SerialEventHandler_for_kernel(){}
        void onDataReceived(char data) override
        {
            if (data != 0 && m_shell != 0) {
                m_shell->handle_key(data);
            }
        }
};