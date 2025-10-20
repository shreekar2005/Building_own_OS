#include "kmouse"

uint16_t MouseDriver::old_char_under_cursor;
// DO NOT CHANGE THESE -1 INITIALIZATIONS (THAT WILL LEAD TO BUG OF "GHOST MOUSE POINTER")
int8_t MouseDriver::__mouse_x_=-1;
int8_t MouseDriver::__mouse_y_=-1;

// Constructor: Initialize the new state variable
MouseDriver::MouseDriver(InterruptManager* interrupt_manager)
: InterruptHandler(0x2C, interrupt_manager), 
  dataPort(0x60), 
  commandPort(0x64)
{
    while(commandPort.read() & 1) dataPort.read();

    offset=0;
    buttons=0;
    __mouse_x_=40;
    __mouse_y_=12;
    uint16_t* video_memory=(uint16_t*) 0xb8000;
    old_char_under_cursor = video_memory[80*__mouse_y_+__mouse_x_];
    video_memory[80*__mouse_y_+__mouse_x_] = (old_char_under_cursor & 0x0FFF) | 0x9000;

    commandPort.write(0xA8); // activate communication for mouse
    commandPort.write(0x20); // get current state
    uint8_t status = dataPort.read() | 2; // set LSB and clear 5th bit
    commandPort.write(0x60); // set state
    dataPort.write(status);
    commandPort.write(0xD4);
    dataPort.write(0xF4);
    dataPort.read();
}

MouseDriver::~MouseDriver(){}

uint32_t MouseDriver::handleInterrupt(uint32_t esp){
    uint8_t status = commandPort.read();
    if(!(status&0x20)) return esp;
    
    buffer[offset] = dataPort.read();
    offset = (offset+1)%3;

    if(offset==0){
        // printf("%hhi ", buffer[0]);
        static uint16_t* video_memory=(uint16_t*) 0xb8000;

        video_memory[80*__mouse_y_+__mouse_x_] = old_char_under_cursor;

        // __mouse_x_+=buffer[1];
        if(buffer[1]>(int8_t)0) __mouse_x_++;
        if(buffer[1]<(int8_t)0) __mouse_x_--;
        if(__mouse_x_<0) __mouse_x_=0;
        if(__mouse_x_>=80) __mouse_x_=79;

        // y-=buffer[2];
        if(buffer[2]>(int8_t)0) __mouse_y_--;
        if(buffer[2]<(int8_t)0) __mouse_y_++;
        if(__mouse_y_<0) __mouse_y_=0;
        if(__mouse_y_>=25) __mouse_y_=24;

        // Check for Left Button
        if (buffer[0] & 0x01) {
            update_cursor(__mouse_x_,__mouse_y_);
        }
        // Check for Right Button
        else if (buffer[0] & 0x02) {
        }
        // Check for Middle Button
        else if (buffer[0] & 0x04) {
        }
        
        old_char_under_cursor = video_memory[80*__mouse_y_+__mouse_x_];

        video_memory[80*__mouse_y_+__mouse_x_] = (old_char_under_cursor & 0x0FFF) | 0x9000;

    }

    return esp;
}