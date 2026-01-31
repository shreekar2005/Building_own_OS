#ifndef _OSOS_BASIC_KIOSTREAM_H
#define _OSOS_BASIC_KIOSTREAM_H

#include "basic/ktypes.hpp"
#include "essential/kstdarg.hpp"

namespace basic
{
/// @brief Clears the entire text mode screen and resets the cursor to (0,0).
void clearScreen();

/// @brief A custom implementation of the standard printf function for writing to both VGA text mode memory and the COM1 serial port.
///
/// This function formats a string and prints it to the screen and serial console, handling various format specifiers,
/// flags, width, precision, and length modifiers. It returns the total number of characters written.
///
/// @param format The null-terminated format string.
/// @param ... Variable arguments corresponding to the format specifiers in the format string.
/// @return The total number of characters written (to both screen and serial).
///
/// @details
/// This function internally uses `printCharStr`, which routes every character to both the
/// VGA buffer (0xb8000) and the serial port (COM1, 0x3F8) simultaneously.
///
/// Supported Format Specifiers:
/// * %c - Character
/// * %s - String
/// * %d, %i - Signed decimal integer
/// * %u - Unsigned decimal integer
/// * %f - Floating point number (double)
/// * %x - Unsigned hexadecimal integer (lowercase)
/// * %X - Unsigned hexadecimal integer (uppercase)
/// * %b - Unsigned binary integer
/// * %o - Unsigned octal integer
/// * %p - Pointer address (formats as 0x prefixed hex)
/// * %% - A literal '%' character
///
/// Supported Flags:
/// * '#' - Alternative form (e.g., 0x for hex, 0b for binary, 0 for octal).
/// * '0' - Zero-padding for width (ignored if precision is specified or '-' flag is used).
///
/// Supported Width:
/// * A number after '%' specifies the minimum field width (e.g., %10d). Padded with spaces by default, or zeros if '0' flag is used.
///
/// Supported Precision:
/// * A period followed by a number (e.g., %.4f, %.8X).
/// * For floats: specifies the number of digits after the decimal point.
/// * For integers: specifies the minimum number of digits to print (padded with leading zeros).
///
/// Supported Length Modifiers:
/// * h  - short (for d, i, u, x, X, o, b)
/// * hh - char (for d, i, u, x, X, o, b)
/// * l  - long (for d, i, u, x, X, o, b)
/// * ll - long long (for d, i, u, x, X, o, b)
int printf(const char *format, ...);

/// @brief Enables the text mode cursor and sets its shape.
/// @param cursor_start The starting scanline for the cursor block.
/// @param cursor_end The ending scanline for the cursor block.
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

/// @brief Updates the position of the text mode cursor.
/// @param x The new x-coordinate (column).
/// @param y The new y-coordinate (row).
void update_cursor(int x, int y);

/// @brief Disables the text mode cursor.
void disable_cursor();

}
    
#endif 