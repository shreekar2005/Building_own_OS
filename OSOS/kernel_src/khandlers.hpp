#ifndef KHANDLERS_HPP
#define KHANDLERS_HPP

#include "driver/kkeyboard.hpp"
#include "driver/kmouse.hpp"
#include "driver/kserial.hpp"
#include "kshell.hpp"

// Keyboard Handler
class KeyboardEventHandler_for_kernel : public driver::KeyboardEventHandler
{
    private:
        KernelShell* m_shell;
    public:
        KeyboardEventHandler_for_kernel(KernelShell* shell) : m_shell(shell) {}
        
        void onKeyDown(char ascii) override {
            if (ascii != 0 && m_shell != 0) {
                m_shell->on_key_pressed(ascii);
            }
        }
        void onKeyUp(char ascii) override { (void)ascii; }
};

// Mouse Handler
class MouseEventHandler_for_kernel : public driver::MouseEventHandler{
    public:
        void onMouseDown(uint8_t button) override {
            (void)button;
            basic::update_cursor(driver::MouseDriver::__mouse_x_, driver::MouseDriver::__mouse_y_);
        }
        void onMouseUp(uint8_t button) override { (void)button; }
        
        void onMouseMove(int8_t delta_x, int8_t delta_y) override {
            static uint16_t* video_memory = (uint16_t*) 0xb8000;
            static float deltaTod=0.2;
            static float dx=0, dy=0;
            dx+=delta_x*deltaTod;
            dy+=delta_y*deltaTod;

            if (driver::MouseDriver::__mouse_x_ >= 0 && driver::MouseDriver::__mouse_y_ >= 0) {
                video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_] = driver::MouseDriver::old_char_under_mouse_pointer;
            }

            if(dx > 1) { driver::MouseDriver::__mouse_x_++; dx=0; }
            else if(dx < -1) { driver::MouseDriver::__mouse_x_--; dx=0; }
            
            if(dy > 1) { driver::MouseDriver::__mouse_y_++; dy=0; }
            else if(dy < -1) { driver::MouseDriver::__mouse_y_--; dy=0; }

            if(driver::MouseDriver::__mouse_x_ < 0) driver::MouseDriver::__mouse_x_ = 0;
            if(driver::MouseDriver::__mouse_x_ >= 80) driver::MouseDriver::__mouse_x_ = 79;
            if(driver::MouseDriver::__mouse_y_ < 0) driver::MouseDriver::__mouse_y_ = 0;
            if(driver::MouseDriver::__mouse_y_ >= 25) driver::MouseDriver::__mouse_y_ = 24;

            driver::MouseDriver::old_char_under_mouse_pointer = video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_];
            video_memory[80 * driver::MouseDriver::__mouse_y_ + driver::MouseDriver::__mouse_x_] = driver::MouseDriver::mouse_block_video_mem_value(driver::MouseDriver::old_char_under_mouse_pointer, MOUSE_POINTER_COLOR);
        }
};

// Serial Handler
class SerialEventHandler_for_kernel : public driver::SerialEventHandler{
    private:
        KernelShell* m_shell;
    public:
        SerialEventHandler_for_kernel(KernelShell* shell) : m_shell(shell) {}
        void onDataReceived(char data) override {
            if (data != 0 && m_shell != 0) m_shell->on_key_pressed(data);
        }
};

#endif