#include <cstdint>
#include "kicxxabi"
#include "multiboot" // header for handling multiboot info (provided by grub)
#include "kiostream"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "kmemory" // for using printMemoryMap()
#include "kgdt" // for global discripter table
#include "kinterrupt"
#include "kkeyboard"
#include "kmouse"
#include "kdriver"

class KeyboardEventHandler_for_kernel : public KeyboardEventHandler{
    public:
        KeyboardEventHandler_for_kernel(){}
        ~KeyboardEventHandler_for_kernel(){}
        void onKeyDown(char ascii) override {
            if (ascii != 0) {
                printf("%c", ascii);
            }
        }
        void onKeyUp(char ascii) override {
            (void) ascii;
        }
};

class MouseEventHandler_for_kernel : public MouseEventHandler{
    public:
        MouseEventHandler_for_kernel(){}
        ~MouseEventHandler_for_kernel(){}
        // Default implementations for the handler. A specific handler can override these.
        void onMouseDown(uint8_t button) override {
            (void) button;
            update_cursor(MouseDriver::__mouse_x_, MouseDriver::__mouse_y_);
            // printf("Mouse Down: %d\n", button);
        }

        void onMouseUp(uint8_t button) override {
            (void) button;
            // printf("Mouse Up: %d\n", button);
        }

        void onMouseMove(int8_t delta_x, int8_t delta_y) override {
            static uint16_t* video_memory = (uint16_t*) 0xb8000;
            // Restore the character that was previously under the mouse pointer
            if (MouseDriver::__mouse_x_ >= 0 && MouseDriver::__mouse_y_ >= 0) {
                video_memory[80 * MouseDriver::__mouse_y_ + MouseDriver::__mouse_x_] = MouseDriver::old_char_under_mouse_pointer;
            }
            // Update mouse coordinates based on delta
            if(delta_x > 0) MouseDriver::__mouse_x_++;
            else if(delta_x < 0) MouseDriver::__mouse_x_--;
            
            if(delta_y > 0) MouseDriver::__mouse_y_++;
            else if(delta_y < 0) MouseDriver::__mouse_y_--;

            // Clamp coordinates to screen boundaries
            if(MouseDriver::__mouse_x_ < 0) MouseDriver::__mouse_x_ = 0;
            if(MouseDriver::__mouse_x_ >= 80) MouseDriver::__mouse_x_ = 79;
            if(MouseDriver::__mouse_y_ < 0) MouseDriver::__mouse_y_ = 0;
            if(MouseDriver::__mouse_y_ >= 25) MouseDriver::__mouse_y_ = 24;

            // Save the character at the new position and draw the pointer over it
            MouseDriver::old_char_under_mouse_pointer = video_memory[80 * MouseDriver::__mouse_y_ + MouseDriver::__mouse_x_];
            video_memory[80 * MouseDriver::__mouse_y_ + MouseDriver::__mouse_x_] = MouseDriver::mouse_block_video_mem_value(MouseDriver::old_char_under_mouse_pointer, MOUSE_POINTER_COLOR);
        }
};

extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    // int a=1/0;
    __callConstructors();
    enable_cursor(0,15); // (0,15) is for blinking block
    __clearScreen();
    //-------------Global Descriptor Table -------------
    GDT osos_GDT;
    osos_GDT.installTable();
    //------------Interrupt Descriptor Table and Drivers -------------
    InterruptManager osos_InterruptManager(&osos_GDT);
    osos_InterruptManager.installTable();
    
    //------------Creating object of drivers so that they will handle their corresponding Interrupts------------
    DriverManager driverManager;

        KeyboardEventHandler_for_kernel keyboardEventHandler_for_kernel;
        KeyboardDriver keyboard(&osos_InterruptManager,&keyboardEventHandler_for_kernel);
        driverManager.addDriver(&keyboard);

        MouseEventHandler_for_kernel mouseEventHandler_for_kernel;
        MouseDriver mouse(&osos_InterruptManager, &mouseEventHandler_for_kernel);
        driverManager.addDriver(&mouse);

    driverManager.activateAll();

    InterruptManager::activate();
    
    GDT::printLoadedTableHeader();
    InterruptManager::printLoadedTableHeader();
    printf("HELLO FROM OSOS :)\n");
    while (true){};

    disable_cursor();
    __cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}


