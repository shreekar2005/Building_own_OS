#include "driver/kkeyboard.hpp"
#include "basic/kiostream.hpp"
#include "driver/kmouse.hpp"

using namespace driver;

KeyboardEventHandler::KeyboardEventHandler(){}
KeyboardEventHandler::~KeyboardEventHandler(){}


KeyboardDriver::KeyboardDriver(hardware_communication::InterruptManager* interrupt_manager, KeyboardEventHandler* keyboardEventHandler)
: hardware_communication::InterruptHandler(0x21, interrupt_manager), 
  dataPort(0x60), 
  commandPort(0x64),
  shift_pressed(false),
  caps_on(false),
  waiting_for_led_ack(false)
{
    this->keyboardEventHandler=keyboardEventHandler;
}

KeyboardDriver::~KeyboardDriver(){}

uint32_t KeyboardDriver::handleInterrupt(uint32_t esp)
{
    // Unshifted keys
    static const char scancode_no_shift[] = {
        0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
    };

    // Shifted keys
    static const char scancode_shifted[] = {
        0,   0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
    };

    uint8_t scancode = dataPort.read();
    if(keyboardEventHandler==0) return esp; //if dont have any handler just return


    if (this->waiting_for_led_ack) {
        if (scancode == 0xFA) { // Got ACK
            dataPort.write(this->led_byte_to_send);
            this->waiting_for_led_ack = false;
        }
        // If we get something else, the keyboard is out of sync.
        // We'll just drop out of the ACK-waiting state and process
        // the scancode normally (by falling through).
        else {
             this->waiting_for_led_ack = false;
        }

        // If we got the ACK, we're done for this interrupt.
        if (scancode == 0xFA) {
            return esp;
        }
    }


    // Check for key release
    if (scancode & 0x80) {
        scancode -= 0x80; 
        switch(scancode) {
            case 0x2A: // Left Shift Release
            case 0x36: // Right Shift Release
                this->shift_pressed = false;
                break;
            
            // Handle release of all other keys
            default: {
                char ascii = 0;
                if (scancode < sizeof(scancode_no_shift)) {
                    
                    char base_char = scancode_no_shift[scancode];

                    // Determine the character that was released based on the keyboard state
                    if (base_char >= 'a' && base_char <= 'z') {
                        if (this->shift_pressed ^ this->caps_on) {
                            ascii = scancode_shifted[scancode];
                        } else {
                            ascii = base_char;
                        }
                    } else {
                        if (this->shift_pressed) {
                            ascii = scancode_shifted[scancode];
                        } else {
                            ascii = base_char;
                        }
                    }
                    if (ascii != 0) {
                        keyboardEventHandler->onKeyUp(ascii);
                    }
                }
                break;
            }
        }
    } 

    // Check for key press
    else {
        switch(scancode) {
            case 0x2A: // Left Shift Press
            case 0x36: // Right Shift Press
                this->shift_pressed = true;
                break;

            case 0x3A: // Caps Lock Press
                this->caps_on = !this->caps_on; // Toggle the state
                
                // Prepare to send LED update
                this->led_byte_to_send = 0;
                if (this->caps_on) this->led_byte_to_send |= 0x04; // Bit 2 for Caps Lock LED
                
                dataPort.write(0xED); // Send "Set LEDs" command
                this->waiting_for_led_ack = true; // Set state to wait for ACK
                break;

            default: {
                // It's a printable key
                char ascii = 0;
                if (scancode < sizeof(scancode_no_shift)) {
                    
                    char base_char = scancode_no_shift[scancode];

                    // Check if it's an alphabet character
                    if (base_char >= 'a' && base_char <= 'z') {
                        // It's a letter. Apply Shift XOR Caps Lock
                        // (shift ^ caps) = true means capitalize
                        if (this->shift_pressed ^ this->caps_on) {
                            ascii = scancode_shifted[scancode]; // Uppercase
                        } else {
                            ascii = base_char; // Lowercase
                        }
                    } else {
                        // It's not a letter (number, symbol, etc.)
                        // Only Shift applies
                        if (this->shift_pressed) {
                            ascii = scancode_shifted[scancode];
                        } else {
                            ascii = base_char;
                        }
                    }
                    keyboardEventHandler->onKeyDown(ascii);
                }
                break;
            }
        }
    }

    return esp;
}


void KeyboardDriver::activate()
{
    while(commandPort.read() & 1) dataPort.read();
    commandPort.write(0xAE); // activate communication for keyboard
    commandPort.write(0x20); // get current state
    uint8_t status = (dataPort.read() | 1) & ~0x10; // set LSB and clear 5th bit
    commandPort.write(0x60); // set state
    dataPort.write(status);
    dataPort.write(0xF4);
    basic::printf("Keyboard Driver activated!\n");
}

int KeyboardDriver::reset(){return 0;}

void KeyboardDriver::deactivate(){}