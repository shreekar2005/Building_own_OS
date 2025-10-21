#include <cstdint>
#include "essential/kicxxabi"
#include "basic/multiboot" // header for handling multiboot info (provided by grub)
#include "basic/kiostream"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "basic/kmemory" // for using printMemoryMap()
#include "essential/kgdt" // for global discripter table
#include "hardware_communication/kinterrupt"
#include "driver/kkeyboard"
#include "driver/kmouse"
#include "driver/kdriver"


class KeyboardEventHandler_for_kernel : public driver::KeyboardEventHandler{
    public:
        KeyboardEventHandler_for_kernel(){}
        ~KeyboardEventHandler_for_kernel(){}
        void onKeyDown(char ascii) override {
            if (ascii != 0) {
                basic::printf("%c", ascii);
            }
        }
        void onKeyUp(char ascii) override {
            (void) ascii;
        }
};

class MouseEventHandler_for_kernel : public driver::MouseEventHandler{
    public:
        MouseEventHandler_for_kernel(){}
        ~MouseEventHandler_for_kernel(){}
        // Default implementations for the handler. A specific handler can override these.
        void onMouseDown(uint8_t button) override {
            (void) button;
            basic::update_cursor(driver::MouseDriver::__mouse_x_, driver::MouseDriver::__mouse_y_);
            // printf("Mouse Down: %d\n", button);
        }

        void onMouseUp(uint8_t button) override {
            (void) button;
            // printf("Mouse Up: %d\n", button);
        }

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

extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    // int a=1/0;
    essential::__callConstructors();
    basic::enable_cursor(0,15); // (0,15) is for blinking block
    basic::__clearScreen();
    //-------------Global Descriptor Table -------------
    essential::GDT osos_GDT;
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
    
    essential::GDT::printLoadedTableHeader();
    hardware_communication::InterruptManager::printLoadedTableHeader();
    basic::printf("HELLO FROM OSOS :)\n");
    while (true){};

    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}


