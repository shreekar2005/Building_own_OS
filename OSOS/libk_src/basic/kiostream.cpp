#include "basic/kiostream.hpp"
#include "driver/kmouse.hpp"

using hardware_communication::Port8Bit;

namespace basic
{

static int cursor_x_ = 0;
static int cursor_y_ = 0;

#define MAGIC_WIDTH 80
#define MAGIC_HEIGHT 25
#define TAB_WIDTH 4 

static Port8Bit vgaIndexPort(0x3D4);
static Port8Bit vgaDataPort(0x3D5);


/// @brief Checks if the serial port's transmit buffer is empty (COM1).
static bool is_serial_ready()
{
    // Check 6th bit (0x20) of Line Status Port (0x3FD)
    Port8Bit lineStatusPort(0x3FD);
    return (lineStatusPort.read() & 0x20); // 0x20 = (1 << 5)
}

/// @brief Writes a single character to the COM1 serial port.
static void write_serial_char(char c)
{
    Port8Bit dataPort(0x3F8); // COM1 Data Port
    
    // Handle newline: serial port needs CRLF (\r\n)
    if (c == '\n') {
        while (is_serial_ready() == 0); // Wait for port to be ready
        dataPort.write((uint8_t)'\r'); // Send carriage return
    }
    
    while (is_serial_ready() == 0); // Wait for port to be ready
    dataPort.write((uint8_t)c);    // Send the character
}



void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    vgaIndexPort.write(0x0A);
    vgaDataPort.write((vgaDataPort.read() & 0xC0) | cursor_start);
    
    vgaIndexPort.write(0x0B);
    vgaDataPort.write((vgaDataPort.read() & 0xE0) | cursor_end);
}


void update_cursor(int x, int y)
{
    cursor_x_=x;
    cursor_y_=y;
    uint16_t pos = y * 80 + x;
    vgaIndexPort.write(0x0F);
    vgaDataPort.write((uint8_t)(pos & 0xFF));
    vgaIndexPort.write(0x0E);
    vgaDataPort.write((uint8_t)((pos >> 8) & 0xFF));
}


void disable_cursor()
{
    vgaIndexPort.write(0x0A);
    vgaDataPort.write(0x20);
}


/// @brief Internal function to print a null-terminated string to video memory AND serial.
/// @param str The string to print.
static void printCharStr(const char *str)
{
    uint16_t *video_memory = (uint16_t *)0xb8000;
    // FOR TEXT : Attribute for Gray-White (0x7) text on a Black (0x0) background (last 2 bytes (LSBs) are not used for color).
    const uint16_t color_attribute = 0x0700;

    // remove mouse pointer
    int mouse_offset = driver::MouseDriver::__mouse_y_ * MAGIC_WIDTH + driver::MouseDriver::__mouse_x_;
    if(mouse_offset>=0) video_memory[mouse_offset] = driver::MouseDriver::old_char_under_mouse_pointer;


    for (int i = 0; str[i] != '\0'; i++)
    {
        write_serial_char(str[i]);
        if (str[i] == '\n')
        {
            cursor_y_++;
            cursor_x_ = 0;
        }
        else if (str[i] == '\b')
        {
            if (cursor_x_ > 0)
            {
                cursor_x_--;
                int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
                video_memory[offset] = color_attribute | ' ';
            }
        }
        else if (str[i] == '\r')
        {
            cursor_x_ = 0;
        }
        else if (str[i] == '\t')
        {
            cursor_x_ = cursor_x_ + (TAB_WIDTH - (cursor_x_ % TAB_WIDTH));
        }
        else
        {
            int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
            video_memory[offset] = color_attribute | str[i];
            cursor_x_++;
        }

        if (cursor_x_ >= MAGIC_WIDTH)
        {
            cursor_y_++;
            cursor_x_ = 0;
        }

        if (cursor_y_ >= MAGIC_HEIGHT)
        {
            for (int y = 0; y < (MAGIC_HEIGHT - 1); y++)
            {
                for (int x = 0; x < MAGIC_WIDTH; x++)
                {
                    int current_offset = y * MAGIC_WIDTH + x;
                    int next_line_offset = (y + 1) * MAGIC_WIDTH + x;
                    video_memory[current_offset] = video_memory[next_line_offset];
                }
            }

            int last_line_offset_start = (MAGIC_HEIGHT - 1) * MAGIC_WIDTH;
            for (int x = 0; x < MAGIC_WIDTH; x++)
            {
                video_memory[last_line_offset_start + x] = color_attribute | ' ';
            }

            cursor_y_ = MAGIC_HEIGHT - 1;
            cursor_x_ = 0;
        }
    }

    // add mouse pointer again
    driver::MouseDriver::old_char_under_mouse_pointer = video_memory[mouse_offset];
    if(mouse_offset>=0) video_memory[mouse_offset] = driver::MouseDriver::mouse_block_video_mem_value(driver::MouseDriver::old_char_under_mouse_pointer, MOUSE_POINTER_COLOR);

}


void clearScreen()
{
    unsigned short *video_memory = (unsigned short *)0xb8000;
    unsigned short blank = (0x07 << 8) | ' ';
    int screenSize = MAGIC_WIDTH * MAGIC_HEIGHT;

    // This part is correct for clearing the VGA screen
    for (int i = 0; i < screenSize; i++)
    {
        video_memory[i] = blank;
    }

    cursor_x_ = 0;
    cursor_y_ = 0;
    update_cursor(cursor_x_, cursor_y_);

    // ANSI escape sequence to clear the serial terminal
    // \e[2J = Clear entire screen
    // \e[H  = Move cursor to home (top-left)
    const char* clear_sequence = "\e[2J\e[H";
    for (int i = 0; clear_sequence[i] != '\0'; i++)
    {
        write_serial_char(clear_sequence[i]);
    }
}

/// @brief Reverses a string in place.
/// @param str The string to reverse.
/// @param length The length of the string.
static void reverse(char *str, int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/// @brief Converts an unsigned long long integer to a string.
/// @param n The number to convert.
/// @param buffer The output buffer to store the string.
/// @param base The numerical base (e.g., 2, 8, 10, 16).
/// @param is_signed Whether the original number was signed (for handling negative sign).
/// @param uppercase Whether to use uppercase letters for bases > 10.
static void ullToString(unsigned long long n, char *buffer, int base, int is_signed, int uppercase)
{
    int i = 0;
    int isNegative = 0;

    if (n == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (is_signed && (long long)n < 0)
    {
        isNegative = 1;
        n = -(long long)n;
    }

    while (n != 0)
    {
        int rem = n % base;
        buffer[i++] = (rem > 9) ? ((rem - 10) + (uppercase ? 'A' : 'a')) : (rem + '0');
        n = n / base;
    }

    if (isNegative)
    {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';
    reverse(buffer, i);
}

/// @brief Calculates the power of a number.
/// @param base The base.
/// @param exp The exponent.
/// @return The result of base raised to the power of exp.
static double power(double base, int exp)
{
    double res = 1.0;
    for (int i = 0; i < exp; ++i)
        res *= base;
    return res;
}

/// @brief Converts a double-precision floating point number to a string.
/// @param d The double to convert.
/// @param buffer The output buffer to store the string.
/// @param precision The number of digits after the decimal point.
static void doubleToString(double d, char *buffer, int precision)
{
    if (precision < 0)
        precision = 6;
    char *ptr = buffer;

    if (d < 0)
    {
        *ptr++ = '-';
        d = -d;
    }
    
    unsigned long long int_part = (unsigned long long)d;
    double frac_part = d - (double)int_part;

    ullToString(int_part, ptr, 10, 0, 0);
    while (*ptr) ptr++;

    *ptr++ = '.';
    
    unsigned long long frac_as_ull = (unsigned long long)(frac_part * power(10, precision) + 0.5);
    
    char frac_buffer[32];
    ullToString(frac_as_ull, frac_buffer, 10, 0, 0);

    int frac_len = 0;
    while(frac_buffer[frac_len] != '\0') frac_len++;
    
    int padding = precision - frac_len;
    for (int i = 0; i < padding; i++)
        *ptr++ = '0';
    
    char* frac_ptr = frac_buffer;
    while(*frac_ptr) *ptr++ = *frac_ptr++;

    *ptr = '\0';
}

/// @brief Prints a hexadecimal number with leading zeros.
/// @param n The number to print.
/// @param digits The total number of digits to print (padded with zeros).
static void printHex(uintptr_t n, int digits)
{
    char buffer[32];
    ullToString(n, buffer, 16, 0, 1); // Use uppercase for pointers typically

    int len = 0;
    while (buffer[len] != '\0') {
        len++;
    }

    for (int i = 0; i < digits - len; i++) {
        printCharStr("0");
    }
    printCharStr(buffer);
}


int printf(const char *format, ...)
{
    int chars_written = 0;

    va_list args;
    va_start(args, format);

    char buffer[128];
    char char_str[2] = {0, 0};

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            i++;
            
            int use_alternative_form = 0;
            int zero_pad = 0;
            int width = 0;
            int precision = -1;
            
            // Flags
            if (format[i] == '#') {
                use_alternative_form = 1;
                i++;
            }
            if (format[i] == '0') {
                zero_pad = 1;
                i++;
            }

            // Width
            while (format[i] >= '0' && format[i] <= '9') {
                width = width * 10 + (format[i] - '0');
                i++;
            }

            // Precision
            if (format[i] == '.') {
                i++;
                precision = 0;
                while (format[i] >= '0' && format[i] <= '9') {
                    precision = precision * 10 + (format[i] - '0');
                    i++;
                }
                zero_pad = 0;
            }

            // Length Modifiers
            int is_long = 0, is_long_long = 0, is_short = 0, is_char = 0;
            if (format[i] == 'l') {
                is_long = 1; i++;
                if (format[i] == 'l') { is_long_long = 1; is_long = 0; i++; }
            } else if (format[i] == 'h') {
                is_short = 1; i++;
                if (format[i] == 'h') { is_char = 1; is_short = 0; i++; }
            }

            // Handle Specifiers
            switch (format[i])
            {
                case 'c':
                    char_str[0] = (char)va_arg(args, int);
                    printCharStr(char_str);
                    chars_written++;
                    break;
                case 's':
                {
                    const char *str = va_arg(args, char *);
                    if (!str) {
                        str = "(null)";
                    }
                    int len = 0;
                    while (str[len]) {
                        len++;
                    }
                    printCharStr(str);
                    chars_written += len;
                    break;
                }
                case 'f':
                {
                    doubleToString(va_arg(args, double), buffer, precision);
                    int len = 0;
                    while(buffer[len]) {
                        len++;
                    }
                    printCharStr(buffer);
                    chars_written += len;
                    break;
                }
                case 'd': case 'i': case 'u': case 'x': case 'X': case 'b': case 'o':
                {
                    unsigned long long val;
                    int base = 10;
                    int uppercase = 0;
                    char sign_char = 0;

                    if (format[i] == 'd' || format[i] == 'i') {
                        long long signed_val;
                        if (is_long_long) signed_val = va_arg(args, long long);
                        else if (is_long) signed_val = va_arg(args, long);
                        else if (is_char) signed_val = (signed char)va_arg(args, int);
                        else if (is_short) signed_val = (short)va_arg(args, int);
                        else signed_val = va_arg(args, int);
                        
                        if (signed_val < 0) {
                            sign_char = '-';
                            val = -signed_val;
                        } else {
                            val = signed_val;
                        }
                    } else {
                        if (is_long_long) val = va_arg(args, unsigned long long);
                        else if (is_long) val = va_arg(args, unsigned long);
                        else if (is_char) val = (unsigned char)va_arg(args, unsigned int);
                        else if (is_short) val = (unsigned short)va_arg(args, unsigned int);
                        else val = va_arg(args, unsigned int);
                    }

                    switch(format[i]) {
                        case 'x': base = 16; break;
                        case 'X': base = 16; uppercase = 1; break;
                        case 'b': base = 2; break;
                        case 'o': base = 8; break;
                    }
                    
                    ullToString(val, buffer, base, 0, uppercase);

                    const char* prefix = "";
                    if (use_alternative_form && val != 0) {
                        switch(format[i]) {
                            case 'x': prefix = "0x"; break;
                            case 'X': prefix = "0X"; break;
                            case 'b': prefix = "0b"; break;
                            case 'o': prefix = "0"; break;
                        }
                    }
                    
                    int num_len = 0; while(buffer[num_len]) num_len++;
                    int prefix_len = 0; while(prefix[prefix_len]) prefix_len++;
                    
                    int precision_pads = (precision > num_len) ? (precision - num_len) : 0;
                    int total_len = num_len + (sign_char ? 1 : 0) + prefix_len + precision_pads;
                    int width_pads = (width > total_len) ? (width - total_len) : 0;
                    
                    chars_written += width_pads + total_len;

                    if (!zero_pad && width_pads > 0) {
                        for (int j = 0; j < width_pads; j++) printCharStr(" ");
                    }
                    if (sign_char) {
                        char_str[0] = sign_char; printCharStr(char_str);
                    }
                    if (prefix_len > 0) printCharStr(prefix);
                    if (zero_pad && width_pads > 0) {
                        for (int j = 0; j < width_pads; j++) printCharStr("0");
                    }
                    if (precision_pads > 0) {
                        for (int j = 0; j < precision_pads; j++) printCharStr("0");
                    }
                    printCharStr(buffer);
                    break;
                }
                
                case 'p':
                {
                    printCharStr("0x");
                    int hex_digits = sizeof(uintptr_t) * 2;
                    printHex((uintptr_t)va_arg(args, void *), hex_digits);
                    chars_written += 2 + hex_digits;
                    break;
                }
                case '%':
                    printCharStr("%");
                    chars_written++;
                    break;
                default:
                    printCharStr("%");
                    char_str[0] = format[i];
                    printCharStr(char_str);
                    chars_written += 2;
                    break;
            }
        }
        else
        {
            char_str[0] = format[i];
            printCharStr(char_str);
            chars_written++;
        }
    }
    va_end(args);
    update_cursor(cursor_x_, cursor_y_);
    return chars_written;
}

}
