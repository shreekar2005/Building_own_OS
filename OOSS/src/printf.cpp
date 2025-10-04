// You will need this for handling variable arguments
#include <stdarg.h>

// Forward declarations of your existing functions
#define exC extern "C"
// exC void printCharStr(char* str);
// exC void clearScreen();

// Your existing cursor variables
extern int cursor_x_;
extern int cursor_y_;

/**
 * @brief Prints a null-terminated string to the screen at the current cursor position.
 * This is the most fundamental printing function.
 */
exC void printCharStr(const char *str)
{
    // The video memory for text mode starts at address 0xb8000
    unsigned short *VideoMemory = (unsigned short *)0xb8000;

    for (int i = 0; str[i] != '\0'; i++)
    {
        // Handle newline characters
        if (str[i] == '\n')
        {
            cursor_y_++;
            cursor_x_ = 0;
        }
        else
        {
            // Calculate the memory offset for the character
            int offset = cursor_y_ * 80 + cursor_x_;
            // Set the character (low byte) but keep the color (high byte)
            VideoMemory[offset] = (VideoMemory[offset] & 0xFF00) | str[i];
            cursor_x_++;
        }

        // If the cursor reaches the end of the line, wrap to the next one
        if (cursor_x_ >= 80)
        {
            cursor_y_++;
            cursor_x_ = 0;
        }

        // If the cursor reaches the bottom of the screen, scroll everything up
        if (cursor_y_ >= 25)
        {
            for (int y = 0; y < 24; y++)
            {
                for (int x = 0; x < 80; x++)
                {
                    VideoMemory[y * 80 + x] = VideoMemory[(y + 1) * 80 + x];
                }
            }
            // Clear the last line
            for (int x = 0; x < 80; x++)
            {
                VideoMemory[24 * 80 + x] = (VideoMemory[24 * 80 + x] & 0xFF00) | ' ';
            }
            cursor_y_ = 24;
        }
    }
}

/**
 * @brief Clears the entire screen and resets the cursor to the top-left.
 */
exC void clearScreen()
{
    unsigned short *VideoMemory = (unsigned short *)0xb8000;
    // 0x07 is the color code for Light Grey text on a Black background
    unsigned short blank = (0x07 << 8) | ' ';
    int screenSize = 80 * 25;

    for (int i = 0; i < screenSize; i++)
    {
        VideoMemory[i] = blank;
    }

    cursor_x_ = 0;
    cursor_y_ = 0;
}

// ===================================================================================
// SECTION 2: PRINTF IMPLEMENTATION
// Description: The full printf function and its helper utilities.
//              This code requires the functions from Section 1 to work.
// ===================================================================================

// Helper function to reverse a string in place
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

// Helper function to convert an integer to a string
static void intToString(int n, char *buffer)
{
    int i = 0;
    int isNegative = 0;

    if (n == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (n < 0)
    {
        isNegative = 1;
        unsigned int val = -n; // Use unsigned to handle INT_MIN correctly
        n = val;
    }

    while (n != 0)
    {
        buffer[i++] = (n % 10) + '0';
        n = n / 10;
    }

    if (isNegative)
    {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}

// Helper function to convert an unsigned integer to a hexadecimal string
static void hexToString(unsigned int n, char *buffer)
{
    int i = 0;
    if (n == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    while (n != 0)
    {
        unsigned int rem = n % 16;
        buffer[i++] = (rem < 10) ? (rem + '0') : ((rem - 10) + 'A');
        n = n / 16;
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}

/**
 * @brief A kernel-level printf function to print formatted strings to the screen.
 * Supports %c, %s, %d, %i, %x, %p, and %%.
 */
exC void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[32];
    char char_str[2] = {0};

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            i++;
            switch (format[i])
            {
            case 'c':
                printCharStr((char_str[0] = (char)va_arg(args, int), char_str));
                break;
            case 's':
                printCharStr(va_arg(args, char *));
                break;
            case 'd':
            case 'i':
                intToString(va_arg(args, int), buffer);
                printCharStr(buffer);
                break;
            case 'x':
                hexToString(va_arg(args, unsigned int), buffer);
                printCharStr("0x");
                printCharStr(buffer);
                break;
            case 'p':
                hexToString((unsigned int)va_arg(args, void *), buffer);
                printCharStr("0x");
                printCharStr(buffer);
                break;
            case '%':
                printCharStr("%");
                break;
            default:
                printCharStr("%");
                printCharStr((char_str[0] = format[i], char_str));
                break;
            }
        }
        else
        {
            printCharStr((char_str[0] = format[i], char_str));
        }
    }

    va_end(args);
}
