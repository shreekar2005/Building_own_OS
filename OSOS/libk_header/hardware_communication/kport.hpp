#ifndef _OSOS_HARDWARECOMMUNICATION_KPORT_H 
#define _OSOS_HARDWARECOMMUNICATION_KPORT_H

#include <cstdint>
#include "basic/kiostream.hpp"

namespace hardware_communication
{
/// @brief Base class for hardware I/O port communication.
class Port {
protected:
    uint16_t portnumber;
    explicit Port(uint16_t portnumber);
public:
    virtual ~Port();
};

/// @brief Represents an 8-bit (byte-sized) I/O Port.
class Port8Bit : public Port {
public:
    explicit Port8Bit(uint16_t portnumber);
    virtual ~Port8Bit() override;

    /// @brief Writes an 8-bit value to the I/O port.
    /// @param data The 8-bit value to write.
    virtual void write(uint8_t data);

    /// @brief Reads an 8-bit value from the I/O port.
    /// @return The 8-bit value read from the port.
    uint8_t read();
};

/// @brief Represents a Port8Bit with a forced delay on writes.
class Port8BitSlow final : public Port8Bit {
public:
    explicit Port8BitSlow(uint16_t portnumber);
    ~Port8BitSlow() override;

    /// @brief Writes an 8-bit value to the I/O port with a delay.
    /// @param data The 8-bit value to write.
    void write(uint8_t data) override;
    //uint8_t read(); inherited from Port8Bit
};

/// @brief Represents a 16-bit (word-sized) I/O Port.
class Port16Bit final : public Port {
public:
    explicit Port16Bit(uint16_t portnumber);
    ~Port16Bit() override;

    /// @brief Writes a 16-bit value to the I/O port.
    /// @param data The 16-bit value to write.
    void write(uint16_t data);

    /// @brief Reads a 16-bit value from the I/O port.
    /// @return The 16-bit value read from the port.
    uint16_t read();
};


/// @brief Represents a 32-bit (double-word-sized) I/O Port.
class Port32Bit final : public Port {
public:
    explicit Port32Bit(uint16_t portnumber);
    ~Port32Bit() override;

    /// @brief Writes a 32-bit value to the I/O port.
    /// @param data The 32-bit value to write.

    void write(uint32_t data);
    /// @brief Reads a 32-bit value from the I/O port.
    /// @return The 32-bit value read from the port.
    uint32_t read();
};
}

#endif