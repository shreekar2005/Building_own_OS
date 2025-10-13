#include "kkeyboard"

KeyboardDriver::KeyboardDriver(InterruptManager* interrupt_manager):InterruptHandler(0x21, interrupt_manager), dataPort(0x60), commandPort(0x64){
    while(commandPort.read() & 1) dataPort.read();
    commandPort.write(0xAE); // activate communication for keyboard
    commandPort.write(0x20); // get current state
    uint8_t status = (dataPort.read() | 1) & ~0x10; // set LSB and clear 5th bit
    commandPort.write(0x60); // set state
    dataPort.write(status);
    dataPort.write(0xF4);
}
KeyboardDriver::~KeyboardDriver(){}

uint32_t KeyboardDriver::handleInterrupt(uint32_t esp){
    static const char scancode_to_ascii[] = {
        0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
    };

    uint8_t scancode = dataPort.read();
    if (scancode < sizeof(scancode_to_ascii) && scancode_to_ascii[scancode] != 0) {
        printf("%c", scancode_to_ascii[scancode]);
    }
    return esp;
}