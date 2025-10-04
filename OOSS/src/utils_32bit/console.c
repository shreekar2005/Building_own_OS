#include "console.h"
#include <stdarg.h>

int cursor_x_ = 0;
int cursor_y_ = 0;
#define MAGIC_WIDTH 80
#define MAGIC_HEIGHT 25

void printCharStr(const char *str)
{
    unsigned short *VideoMemory = (unsigned short *)0xb8000;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            cursor_y_++;
            cursor_x_ = 0;
        }
        else
        {
            int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
            VideoMemory[offset] = (VideoMemory[offset] & 0xFF00) | str[i];
            cursor_x_++;
        }

        if (cursor_x_ >= MAGIC_WIDTH)
        {
            cursor_y_++;
            cursor_x_ = 0;
        }

        if (cursor_y_ >= MAGIC_HEIGHT)
        {
            for (int y = 0; y < (MAGIC_HEIGHT-1); y++)
            {
                for (int x = 0; x < MAGIC_WIDTH; x++)
                {
                    VideoMemory[y * MAGIC_WIDTH + x] = VideoMemory[(y + 1) * MAGIC_WIDTH + x];
                }
            }
            for (int x = 0; x < MAGIC_WIDTH; x++)
            {
                VideoMemory[(MAGIC_HEIGHT-1) * MAGIC_WIDTH + x] = (VideoMemory[(MAGIC_HEIGHT-1) * MAGIC_WIDTH + x] & 0xFF00) | ' ';
            }
            cursor_y_ = (MAGIC_HEIGHT-1);
        }
    }
}

void clearScreen()
{
    unsigned short *VideoMemory = (unsigned short *)0xb8000;
    unsigned short blank = (0x07 << 8) | ' ';
    int screenSize = MAGIC_WIDTH * MAGIC_HEIGHT;

    for (int i = 0; i < screenSize; i++)
    {
        VideoMemory[i] = blank;
    }

    cursor_x_ = 0;
    cursor_y_ = 0;
}

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

static void intToString(int n, char *buffer)
{
    int i = 0;
    int isNegative = 0;
    unsigned int u_n;

    if (n == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (n < 0)
    {
        isNegative = 1;
        u_n = -(unsigned int)n;
    }
    else
    {
        u_n = (unsigned int)n;
    }

    while (u_n != 0)
    {
        buffer[i++] = (u_n % 10) + '0';
        u_n = u_n / 10;
    }

    if (isNegative)
    {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}

static void unsignedIntToString(unsigned int n, char *buffer)
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
        buffer[i++] = (n % 10) + '0';
        n = n / 10;
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}

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

static void binaryToString(unsigned int n, char *buffer)
{
    int i = 0;
    if (n == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    while (n > 0)
    {
        buffer[i++] = (n % 2) + '0';
        n = n / 2;
    }
    buffer[i] = '\0';
    reverse(buffer, i);
}

static double power(double base, int exp)
{
    double res = 1.0;
    for (int i = 0; i < exp; ++i)
        res *= base;
    return res;
}

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

    int int_part = (int)d;
    double frac_part = d - (double)int_part;

    char int_buffer[12];
    intToString(int_part, int_buffer);
    for (int i = 0; int_buffer[i] != '\0'; i++)
        *ptr++ = int_buffer[i];

    *ptr++ = '.';

    // Note: The fractional part is also converted to a 32-bit int
    int frac_as_int = (int)(frac_part * power(10, precision) + 0.5);
    char frac_buffer[12];
    intToString(frac_as_int, frac_buffer);

    int frac_len = 0;
    for (frac_len = 0; frac_buffer[frac_len] != '\0'; frac_len++);
    int padding = precision - frac_len;

    for (int i = 0; i < padding; i++)
        *ptr++ = '0';
    for (int i = 0; frac_buffer[i] != '\0'; i++)
        *ptr++ = frac_buffer[i];

    *ptr = '\0';
}

// my printf supports: %c, %s, %d, %i, %u, %f, %x, %b, %p, %%
void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[128];
    char char_str[2] = {0};

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            i++;

            switch (format[i])
            {
            case 'c':
                char_str[0] = (char)va_arg(args, int);
                printCharStr(char_str);
                break;
            case 's':
                printCharStr(va_arg(args, char *));
                break;
            case 'd':
            case 'i':
                intToString(va_arg(args, int), buffer);
                printCharStr(buffer);
                break;
            case 'u':
                unsignedIntToString(va_arg(args, unsigned int), buffer);
                printCharStr(buffer);
                break;
            case 'f':
                doubleToString(va_arg(args, double), buffer, 6);
                printCharStr(buffer);
                break;
            case 'x':
                hexToString(va_arg(args, unsigned int), buffer);
                printCharStr("0x");
                printCharStr(buffer);
                break;
            case 'b':
                binaryToString(va_arg(args, unsigned int), buffer);
                printCharStr("0b");
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
                char_str[0] = format[i];
                printCharStr(char_str);
                break;
            }
        }
        else
        {
            char_str[0] = format[i];
            printCharStr(char_str);
        }
    }
    va_end(args);
}