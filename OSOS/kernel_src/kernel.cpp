#include <cstdint>
#include "basic/multiboot.h" // header for handling multiboot info (provided by grub)
#include "essential/kicxxabi.hpp"
#include "basic/kiostream.hpp"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "basic/kmemory.hpp" // for using printMemoryMap()
#include "essential/kgdt.hpp" // for global discripter table
#include "hardware_communication/kinterrupt.hpp"
#include "driver/kkeyboard.hpp"
#include "driver/kmouse.hpp"
#include "driver/kdriver.hpp"


/// @brief Custom keyboard event handler implementation for the kernel.
class KeyboardEventHandler_for_kernel : public driver::KeyboardEventHandler{
    public:
        /// @brief Construct a new KeyboardEventHandler_for_kernel object
        KeyboardEventHandler_for_kernel(){}
        /// @brief Destroy the KeyboardEventHandler_for_kernel object
        ~KeyboardEventHandler_for_kernel(){}
        /// @brief Handles key down events by printing the ASCII character.
        /// @param ascii The ASCII character of the key that was pressed.
        void onKeyDown(char ascii) override {
            if (ascii != 0) {
                basic::printf("%c", ascii);
            }
        }
        /// @brief Handles key up events. (Does nothing in this implementation).
        /// @param ascii The ASCII character of the key that was released.
        void onKeyUp(char ascii) override {
            (void) ascii;
        }
};

/// @brief Custom mouse event handler implementation for the kernel.
class MouseEventHandler_for_kernel : public driver::MouseEventHandler{
    public:
        /// @brief Construct a new MouseEventHandler_for_kernel object
        MouseEventHandler_for_kernel(){}
        /// @brief Destroy the MouseEventHandler_for_kernel object
        ~MouseEventHandler_for_kernel(){}
        // Default implementations for the handler. A specific handler can override these.
        /// @brief Handles mouse button down events.
        /// @param button The button number that was pressed.
        void onMouseDown(uint8_t button) override {
            (void) button;
            basic::update_cursor(driver::MouseDriver::__mouse_x_, driver::MouseDriver::__mouse_y_);
            // printf("Mouse Down: %d\n", button);
        }

        /// @brief Handles mouse button up events.
        /// @param button The button number that was released.
        void onMouseUp(uint8_t button) override {
            (void) button;
            // printf("Mouse Up: %d\n", button);
        }

        /// @brief Handles mouse movement events and updates the cursor position on the screen.
        /// @param delta_x The change in the x-coordinate.
        /// @param delta_y The change in the y-coordinate.
        void onMouseMove(int8_t delta_x, int8_t delta_y) override {
            static uint16_t* video_memory = (uint16_t*) 0xb8000;
            static float deltaTod=0.2;
            static float dx=0, dy=0;
            dx+=delta_x*deltaTod;
            dy+=delta_y*deltaTod;

            // Restore the character that was previously under the mouse pointer
            if (driver::MouseDriver::__mouse_x_ >= 0 && driver::MouseDriver::__mouse_y_ >= 0) {
                video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_] = driver::MouseDriver::old_char_under_mouse_pointer;
            }
            // Update mouse coordinates based on delta
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

            // Clamp coordinates to screen boundaries
            if(driver::MouseDriver::__mouse_x_ < 0) driver::MouseDriver::__mouse_x_ = 0;
            if(driver::MouseDriver::__mouse_x_ >= 80) driver::MouseDriver::__mouse_x_ = 79;
            if(driver::MouseDriver::__mouse_y_ < 0) driver::MouseDriver::__mouse_y_ = 0;
            if(driver::MouseDriver::__mouse_y_ >= 25) driver::MouseDriver::__mouse_y_ = 24;

            // Save the character at the new position and draw the pointer over it
            driver::MouseDriver::old_char_under_mouse_pointer = video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_];
            video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_] = driver::MouseDriver::mouse_block_video_mem_value(driver::MouseDriver::old_char_under_mouse_pointer, MOUSE_POINTER_COLOR);
        }
};

/// @brief The main entry point for the C++ kernel.
/// @param mbi Pointer to the Multiboot information structure provided by GRUB.
/// @param magicnumber The magic number passed by GRUB to verify boot.
extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    essential::__callConstructors();
    basic::enable_cursor(0,15); // (0,15) is for blinking block
    basic::__clearScreen();
    //-------------Global Descriptor Table -------------
    essential::GDT_Manager osos_GDT;
    osos_GDT.installTable();
    //------------Interrupt Descriptor Table and Drivers -------------
    hardware_communication::InterruptManager osos_InterruptManager(&osos_GDT);
    osos_InterruptManager.installTable();
    
    //------------Creating object of drivers so that they will handle their corresponding Interrupts------------
    driver::DriverManager driverManager;

        KeyboardEventHandler_for_kernel keyboardEventHandler_for_kernel;
        driver::KeyboardDriver keyboard(&osos_InterruptManager,&keyboardEventHandler_for_kernel);
        driverManager.addDriver(&keyboard);

        MouseEventHandler_for_kernel mouseEventHandler_for_kernel;
        driver::MouseDriver mouse(&osos_InterruptManager, &mouseEventHandler_for_kernel);
        driverManager.addDriver(&mouse);

    driverManager.activateAll();

    hardware_communication::InterruptManager::activate();
    
    essential::GDT_Manager::printLoadedTableHeader();
    hardware_communication::InterruptManager::printLoadedTableHeader();
    basic::printf("HELLO FROM OSOS :)\n");
    while (true){};

    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}