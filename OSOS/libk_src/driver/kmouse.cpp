#include "driver/kmouse.hpp"

using namespace driver;

/// @brief Base class for handling mouse events.
MouseEventHandler::MouseEventHandler(){}
/// @brief Destroys the MouseEventHandler object.
MouseEventHandler::~MouseEventHandler(){}


// Static member initialization
uint16_t MouseDriver::old_char_under_mouse_pointer;
// DO NOT CHANGE THESE -1 INITIALIZATIONS (THAT WILL LEAD TO BUG OF "GHOST MOUSE POINTER")
int8_t MouseDriver::__mouse_x_ = -1;
int8_t MouseDriver::__mouse_y_ = -1;

/// @brief Calculates the video memory value for the mouse cursor block.
/// @param current_char The original character attributes and ASCII value at the cursor position.
/// @param mouse_pointer_color The desired background color for the mouse pointer.
/// @return The new 16-bit video memory value representing the character with the new background color.
uint16_t MouseDriver::mouse_block_video_mem_value(uint16_t current_char, uint8_t mouse_pointer_color){
    return (current_char & 0x0FFF) | (mouse_pointer_color << 12);
}

/// @brief Constructs a new MouseDriver object.
/// @param interrupt_manager Pointer to the interrupt manager.
/// @param mouseEventHandler Pointer to the event handler that will process mouse events.
MouseDriver::MouseDriver(hardware_communication::InterruptManager* interrupt_manager, MouseEventHandler* mouseEventHandler)
: hardware_communication::InterruptHandler(0x2C, interrupt_manager), 
  dataPort(0x60), 
  commandPort(0x64)
{
    this->mouseEventHandler = mouseEventHandler;
}

/// @brief Destroys the MouseDriver object.
MouseDriver::~MouseDriver(){}

/// @brief Handles the mouse interrupt (IRQ 12).
/// @param esp The stack pointer from the interrupt context.
/// @return The stack pointer.
uint32_t MouseDriver::handleInterrupt(uint32_t esp)
{
    // Check if the mouse has sent data
    uint8_t status = commandPort.read();
    if(!(status & 0x20)) return esp;
    
    // Read the next byte of the packet
    buffer[offset] = dataPort.read();
    offset = (offset + 1) % 3;

    // When a full 3-byte packet is received
    if(offset == 0){
        if(mouseEventHandler == 0) return esp; // Do nothing without a handler

        // --- Handle Mouse Movement ---
        int8_t delta_x = buffer[1];
        int8_t delta_y = -buffer[2]; // Y-axis is inverted from the mouse's perspective

        if (delta_x != 0 || delta_y != 0) {
            mouseEventHandler->onMouseMove(delta_x, delta_y);
        }

        // --- Handle Mouse Buttons ---
        for(uint8_t i = 0; i < 3; i++) {
            // Check if the state of button 'i' has changed
            if((buffer[0] & (1 << i)) != (buttons & (1 << i))) {
                if(buffer[0] & (1 << i)) {
                    mouseEventHandler->onMouseDown(i + 1); // Left=1, Right=2, Middle=3
                } else {
                    mouseEventHandler->onMouseUp(i + 1);
                }
            }
        }
        buttons = buffer[0]; // Save the current button state for the next interrupt
    }

    return esp;
}


/// @brief Activates the mouse driver.
void MouseDriver::activate()
{
    while(commandPort.read() & 1) dataPort.read();
    offset = 0;
    buttons = 0;
    __mouse_x_ = 40;
    __mouse_y_ = 12;

    static uint16_t* video_memory = (uint16_t*) 0xb8000;
    old_char_under_mouse_pointer = video_memory[80 * __mouse_y_ + __mouse_x_];
    video_memory[80 * __mouse_y_ + __mouse_x_] = mouse_block_video_mem_value(old_char_under_mouse_pointer, MOUSE_POINTER_COLOR);

    // Enable the auxiliary mouse device
    commandPort.write(0xA8); 
    
    // Enable the interrupts for the mouse
    commandPort.write(0x20); 
    uint8_t status = (dataPort.read() | 2); 
    commandPort.write(0x60);
    dataPort.write(status);

    // Set mouse to use default settings and enable packet streaming
    commandPort.write(0xD4);
    dataPort.write(0xF4);
    dataPort.read(); // Acknowledge
    
    basic::printf("Mouse Driver activated!\n");
}

/// @brief Resets the mouse. (Stub)
/// @return Always returns 0.
int MouseDriver::reset(){return 0;}
/// @brief Deactivates the mouse driver. (Stub)
void MouseDriver::deactivate(){}