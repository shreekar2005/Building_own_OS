#include "hardware_communication/kport"

hardware_communication::Port::Port(uint16_t portnumber){
    this->portnumber = portnumber;
}
hardware_communication::Port::~Port(){}

// ################################## 8Bit #####################################

hardware_communication::Port8Bit::Port8Bit(uint16_t portnumber):Port(portnumber){} 
hardware_communication::Port8Bit ::~Port8Bit(){}

void hardware_communication::Port8Bit::write(uint8_t data){
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(portnumber));
}
uint8_t hardware_communication::Port8Bit::read(){
    uint8_t result;
    __asm__ volatile("inb %1, %0" :"=a"(result) : "Nd"(portnumber));
    return result;
}

// ################################## 8BitSlow ##################################

hardware_communication::Port8BitSlow::Port8BitSlow(uint16_t portnumber):hardware_communication::Port8Bit(portnumber){} 
hardware_communication::Port8BitSlow::~Port8BitSlow(){}

void hardware_communication::Port8BitSlow::write(uint8_t data){
    __asm__ volatile("outb %0, %1 \n jmp 1f \n 1: jmp 1f\n 1:" : : "a"(data), "Nd"(portnumber));
}

// ################################## 16Bit #####################################

hardware_communication::Port16Bit::Port16Bit(uint16_t portnumber):Port(portnumber){} 
hardware_communication::Port16Bit::~Port16Bit(){}

void hardware_communication::Port16Bit::write(uint16_t data){
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(portnumber));
}
uint16_t hardware_communication::Port16Bit::read(){
    uint16_t result;
    __asm__ volatile("inw %1, %0" :"=a"(result) : "Nd"(portnumber));
    return result;
}

// ################################## 32Bit #####################################

hardware_communication::Port32Bit::Port32Bit(uint16_t portnumber):Port(portnumber){} 
hardware_communication::Port32Bit::~Port32Bit(){}

void hardware_communication::Port32Bit::write(uint32_t data){
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(portnumber));
}
uint32_t hardware_communication::Port32Bit::read(){
    uint32_t result;
    __asm__ volatile("inl %1, %0" :"=a"(result) : "Nd"(portnumber));
    return result;
}