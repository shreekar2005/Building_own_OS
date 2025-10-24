#include "hardware_communication/kport.hpp"

/// @brief Constructs a new Port object.
/// @param portnumber The 16-bit address of the I/O port.
hardware_communication::Port::Port(uint16_t portnumber){
    this->portnumber = portnumber;
}
/// @brief Destroys the Port object.
hardware_communication::Port::~Port(){}

// ################################## 8Bit #####################################

/// @brief Constructs a new Port8Bit object.
/// @param portnumber The 16-bit address of the I/O port.
hardware_communication::Port8Bit::Port8Bit(uint16_t portnumber):Port(portnumber){} 
/// @brief Destroys the Port8Bit object.
hardware_communication::Port8Bit ::~Port8Bit(){}

/// @brief Writes an 8-bit value to the I/O port.
/// @param data The 8-bit value to write.
void hardware_communication::Port8Bit::write(uint8_t data){
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(portnumber));
}
/// @brief Reads an 8-bit value from the I/O port.
/// @return The 8-bit value read from the port.
uint8_t hardware_communication::Port8Bit::read(){
    uint8_t result;
    __asm__ volatile("inb %1, %0" :"=a"(result) : "Nd"(portnumber));
    return result;
}

// ################################## 8BitSlow ##################################

/// @brief Constructs a new Port8BitSlow object.
/// @param portnumber The 16-bit address of the I/O port.
hardware_communication::Port8BitSlow::Port8BitSlow(uint16_t portnumber):hardware_communication::Port8Bit(portnumber){} 
/// @brief Destroys the Port8BitSlow object.
hardware_communication::Port8BitSlow::~Port8BitSlow(){}

/// @brief Writes an 8-bit value to the I/O port with a delay.
/// @param data The 8-bit value to write.
void hardware_communication::Port8BitSlow::write(uint8_t data){
    __asm__ volatile("outb %0, %1 \n jmp 1f \n 1: jmp 1f\n 1:" : : "a"(data), "Nd"(portnumber));
}

// ################################## 16Bit #####################################

/// @brief Constructs a new Port16Bit object.
/// @param portnumber The 16-bit address of the I/O port.
hardware_communication::Port16Bit::Port16Bit(uint16_t portnumber):Port(portnumber){} 
/// @brief Destroys the Port16Bit object.
hardware_communication::Port16Bit::~Port16Bit(){}

/// @brief Writes a 16-bit value to the I/O port.
/// @param data The 16-bit value to write.
void hardware_communication::Port16Bit::write(uint16_t data){
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(portnumber));
}
/// @brief Reads a 16-bit value from the I/O port.
/// @return The 16-bit value read from the port.
uint16_t hardware_communication::Port16Bit::read(){
    uint16_t result;
    __asm__ volatile("inw %1, %0" :"=a"(result) : "Nd"(portnumber));
    return result;
}

// ################################## 32Bit #####################################

/// @brief Constructs a new Port32Bit object.
/// @param portnumber The 16-bit address of the I/O port.
hardware_communication::Port32Bit::Port32Bit(uint16_t portnumber):Port(portnumber){} 
/// @brief Destroys the Port32Bit object.
hardware_communication::Port32Bit::~Port32Bit(){}

/// @brief Writes a 32-bit value to the I/O port.
/// @param data The 32-bit value to write.
void hardware_communication::Port32Bit::write(uint32_t data){
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(portnumber));
}
/// @brief Reads a 32-bit value from the I/O port.
/// @return The 32-bit value read from the port.
uint32_t hardware_communication::Port32Bit::read(){
    uint32_t result;
    __asm__ volatile("inl %1, %0" :"=a"(result) : "Nd"(portnumber));
    return result;
}