// #include "kiostream"
// #include "kport"

// int cursor_x_ = 0;
// int cursor_y_ = 0;
// #define MAGIC_WIDTH 80
// #define MAGIC_HEIGHT 25
// #define TAB_WIDTH 4 // Standard tab width is 4 characters


// // --- Global Port Objects ---
// Port8Bit keyboardDataPort(0x60);
// Port8Bit keyboardStatusPort(0x64);
// Port8Bit vgaIndexPort(0x3D4);
// Port8Bit vgaDataPort(0x3D5);


// // --- VGA Cursor Control ---

// void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
//     vgaIndexPort.write(0x0A);
//     vgaDataPort.write((vgaDataPort.read() & 0xC0) | cursor_start);
    
//     vgaIndexPort.write(0x0B);
//     vgaDataPort.write((vgaDataPort.read() & 0xE0) | cursor_end);
// }

// void update_cursor(int x, int y) {
//     uint16_t pos = y * 80 + x;
//     vgaIndexPort.write(0x0F);
//     vgaDataPort.write((uint8_t)(pos & 0xFF));
//     vgaIndexPort.write(0x0E);
//     vgaDataPort.write((uint8_t)((pos >> 8) & 0xFF));
// }

// void disable_cursor() {
//     vgaIndexPort.write(0x0A);
//     vgaDataPort.write(0x20);
// }

// // A very basic US QWERTY keyboard layout map.
// // Only handles key presses, not releases.
// static const char scancode_to_ascii[] = {
//     0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
//     '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
//     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
//     '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
//     ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//     0, 0, 0, 0, 0,
// };

// // --- taking keyboard input by polling ---
// char keyboard_input_by_polling() {
//     update_cursor(cursor_x_, cursor_y_);
//     uint8_t scancode;
//     // Wait until the keyboard controller has data for us.
//     while ((keyboardStatusPort.read() & 1) == 0) {}

//     scancode = keyboardDataPort.read();

//     if (scancode < sizeof(scancode_to_ascii) && scancode_to_ascii[scancode] != 0) {
//         printf("%c", scancode_to_ascii[scancode]);
//         return scancode_to_ascii[scancode];
//     }
    
//     return 0; 
// }


// void printCharStr(const char *str)
// {
//     unsigned short *VideoMemory = (unsigned short *)0xb8000;

//     for (int i = 0; str[i] != '\0'; i++)
//     {
//         // Handle special control characters ---

//         if (str[i] == '\n') // Newline character
//         {
//             cursor_y_++;
//             cursor_x_ = 0;
//         }
//         else if (str[i] == '\b') // Backspace character
//         {
//             // Only move back if not at the start of the line
//             if (cursor_x_ > 0)
//             {
//                 cursor_x_--;
//                 // Erase the character at the new position by writing a space
//                 int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
//                 VideoMemory[offset] = (VideoMemory[offset] & 0xFF00) | ' ';
//             }
//         }
//         else if (str[i] == '\r') // Carriage Return character
//         {
//             // Move cursor to the beginning of the current line
//             cursor_x_ = 0;
//         }
//         else if (str[i] == '\t') // Tab character
//         {
//             // Advance cursor to the next tab stop (multiple of TAB_WIDTH)
//             cursor_x_ = cursor_x_ + (TAB_WIDTH - (cursor_x_ % TAB_WIDTH));
//         }
//         else // normal char
//         {
//             int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
//             VideoMemory[offset] = (VideoMemory[offset] & 0xFF00) | str[i];
//             cursor_x_++;
//         }

//         // If cursor goes past the right side, wrap to the next line.
//         if (cursor_x_ >= MAGIC_WIDTH)
//         {
//             cursor_y_++;
//             cursor_x_ = 0;
//         }

//         // If cursor goes off the bottom of the screen, scroll everything up by one line.
//         if (cursor_y_ >= MAGIC_HEIGHT)
//         {
//             // 1. Move each line's content up by one row
//             for (int y = 0; y < (MAGIC_HEIGHT - 1); y++)
//             {
//                 for (int x = 0; x < MAGIC_WIDTH; x++)
//                 {
//                     int current_offset = y * MAGIC_WIDTH + x;
//                     int next_line_offset = (y + 1) * MAGIC_WIDTH + x;
//                     VideoMemory[current_offset] = VideoMemory[next_line_offset];
//                 }
//             }

//             // 2. Clear the last line
//             int last_line_offset_start = (MAGIC_HEIGHT - 1) * MAGIC_WIDTH;
//             for (int x = 0; x < MAGIC_WIDTH; x++)
//             {
//                 VideoMemory[last_line_offset_start + x] = (VideoMemory[last_line_offset_start + x] & 0xFF00) | ' ';
//             }

//             // 3. Reset cursor to the beginning of the now-empty last line
//             cursor_y_ = MAGIC_HEIGHT - 1;
//             cursor_x_ = 0;
//         }
//     }
// }

// void clearScreen()
// {
//     unsigned short *VideoMemory = (unsigned short *)0xb8000;
//     unsigned short blank = (0x07 << 8) | ' ';
//     int screenSize = MAGIC_WIDTH * MAGIC_HEIGHT;

//     for (int i = 0; i < screenSize; i++)
//     {
//         VideoMemory[i] = blank;
//     }

//     cursor_x_ = 0;
//     cursor_y_ = 0;
// }

// static void reverse(char *str, int length)
// {
//     int start = 0;
//     int end = length - 1;
//     while (start < end)
//     {
//         char temp = str[start];
//         str[start] = str[end];
//         str[end] = temp;
//         start++;
//         end--;
//     }
// }

// static void ullToString(unsigned long long n, char *buffer, int base, int is_signed, int uppercase)
// {
//     int i = 0;
//     int isNegative = 0;

//     if (n == 0)
//     {
//         buffer[i++] = '0';
//         buffer[i] = '\0';
//         return;
//     }

//     if (is_signed && (long long)n < 0)
//     {
//         isNegative = 1;
//         n = -(long long)n;
//     }

//     while (n != 0)
//     {
//         int rem = n % base;
//         buffer[i++] = (rem > 9) ? ((rem - 10) + (uppercase ? 'A' : 'a')) : (rem + '0');
//         n = n / base;
//     }

//     if (isNegative)
//     {
//         buffer[i++] = '-';
//     }

//     buffer[i] = '\0';
//     reverse(buffer, i);
// }

// static double power(double base, int exp)
// {
//     double res = 1.0;
//     for (int i = 0; i < exp; ++i)
//         res *= base;
//     return res;
// }

// static void doubleToString(double d, char *buffer, int precision)
// {
//     if (precision < 0)
//         precision = 6;
//     char *ptr = buffer;

//     if (d < 0)
//     {
//         *ptr++ = '-';
//         d = -d;
//     }
    
//     unsigned long long int_part = (unsigned long long)d;
//     double frac_part = d - (double)int_part;

//     ullToString(int_part, ptr, 10, 0, 0);
//     while (*ptr) ptr++;

//     *ptr++ = '.';
    
//     unsigned long long frac_as_ull = (unsigned long long)(frac_part * power(10, precision) + 0.5);
    
//     char frac_buffer[32];
//     ullToString(frac_as_ull, frac_buffer, 10, 0, 0);

//     int frac_len = 0;
//     while(frac_buffer[frac_len] != '\0') frac_len++;
    
//     int padding = precision - frac_len;
//     for (int i = 0; i < padding; i++)
//         *ptr++ = '0';
    
//     char* frac_ptr = frac_buffer;
//     while(*frac_ptr) *ptr++ = *frac_ptr++;

//     *ptr = '\0';
// }

// // A new helper function to handle hex printing with padding
// static void printHex(uintptr_t n, int digits) {
//     char buffer[32]; // Buffer to hold the converted string
    
//     // Convert the number to a hex string
//     ullToString(n, buffer, 16, 0, 0);

//     // Figure out how long the string is
//     int len = 0;
//     while (buffer[len] != '\0') {
//         len++;
//     }

//     // Print the leading zeros
//     for (int i = 0; i < digits - len; i++) {
//         printCharStr("0");
//     }

//     // Print the actual number string
//     printCharStr(buffer);
// }



// /*
//  * my printf supports:
//  * Specifiers: %c, %s, %d, %i, %u, %f, %x, %X, %b, %o, %p, %%
//  * Flags:      '#' (alternative form for o, x, X, b)
//  * Precision:  '.<number>' for %x, %X, and %f (e.g., %.8X, %.2f)
//  * Length:     'l' (long), 'll' (long long)
//  */
// void printf(const char *format, ...)
// {
//     va_list args;
//     va_start(args, format);

//     char buffer[128];
//     char char_str[2] = {0};

//     for (int i = 0; format[i] != '\0'; i++)
//     {
//         if (format[i] == '%')
//         {
//             i++;
            
//             // --- Parsing Flags, Precision, and Length ---
//             int use_alternative_form = 0;
//             int precision = -1; // -1 indicates not specified
            
//             // 1. Flags
//             if (format[i] == '#') {
//                 use_alternative_form = 1;
//                 i++;
//             }

//             // 2. Precision
//             if (format[i] == '.') {
//                 i++;
//                 precision = 0;
//                 while (format[i] >= '0' && format[i] <= '9') {
//                     precision = precision * 10 + (format[i] - '0');
//                     i++;
//                 }
//             }

//             // 3. Length
//             int is_long = 0;
//             int is_long_long = 0;
//             if (format[i] == 'l')
//             {
//                 is_long = 1;
//                 i++;
//                 if (format[i] == 'l')
//                 {
//                     is_long_long = 1;
//                     is_long = 0;
//                     i++;
//                 }
//             }

//             // --- Handling Specifiers ---
//             switch (format[i])
//             {
//             case 'c':
//                 char_str[0] = (char)va_arg(args, int);
//                 printCharStr(char_str);
//                 break;
//             case 's':
//                 printCharStr(va_arg(args, char *));
//                 break;
//             case 'd':
//             case 'i':
//                 if (is_long_long)
//                     ullToString(va_arg(args, long long), buffer, 10, 1, 0);
//                 else if (is_long)
//                     ullToString(va_arg(args, long), buffer, 10, 1, 0);
//                 else
//                     ullToString(va_arg(args, int), buffer, 10, 1, 0);
//                 printCharStr(buffer);
//                 break;
//             case 'u':
//                 if (is_long_long)
//                     ullToString(va_arg(args, unsigned long long), buffer, 10, 0, 0);
//                 else if (is_long)
//                     ullToString(va_arg(args, unsigned long), buffer, 10, 0, 0);
//                 else
//                     ullToString(va_arg(args, unsigned int), buffer, 10, 0, 0);
//                 printCharStr(buffer);
//                 break;
//             case 'f':
//                 // Pass the parsed precision. doubleToString handles the default case.
//                 doubleToString(va_arg(args, double), buffer, precision);
//                 printCharStr(buffer);
//                 break;
//             case 'x':
//             case 'X':
//                 {
//                     unsigned long long val;
//                     if (is_long_long)
//                         val = va_arg(args, unsigned long long);
//                     else if (is_long)
//                         val = va_arg(args, unsigned long);
//                     else
//                         val = va_arg(args, unsigned int);

//                     if (use_alternative_form && val != 0) {
//                         printCharStr("0x");
//                     }
//                     ullToString(val, buffer, 16, 0, (format[i] == 'X'));
                    
//                     int len = 0;
//                     while(buffer[len] != '\0') len++;

//                     // Apply padding if precision is specified
//                     if (precision > len) {
//                         for (int j = 0; j < precision - len; j++) {
//                             printCharStr("0");
//                         }
//                     }

//                     printCharStr(buffer);
//                 }
//                 break;
//             case 'b': // note: %b and %#b are non-standard extensions
//                 {
//                     unsigned long long val;
//                     if (is_long_long)
//                         val = va_arg(args, unsigned long long);
//                     else if (is_long)
//                         val = va_arg(args, unsigned long);
//                     else
//                         val = va_arg(args, unsigned int);
                    
//                     if (use_alternative_form && val != 0) {
//                         printCharStr("0b");
//                     }
//                     ullToString(val, buffer, 2, 0, 0);
//                     printCharStr(buffer);
//                 }
//                 break;
//             case 'o':
//                 {
//                     unsigned long long val;
//                     if (is_long_long)
//                         val = va_arg(args, unsigned long long);
//                     else if (is_long)
//                         val = va_arg(args, unsigned long);
//                     else
//                         val = va_arg(args, unsigned int);
                    
//                     if (use_alternative_form && val != 0) {
//                         printCharStr("0");
//                     }
//                     ullToString(val, buffer, 8, 0, 0);
//                     printCharStr(buffer);
//                 }
//                 break;
//             case 'p':
//                 printCharStr("0x");
//                 if (sizeof(uintptr_t) == 4) { // 32-bit system
//                     printHex((uintptr_t)va_arg(args, void *), 8);
//                 } else if (sizeof(uintptr_t) == 8) { // 64-bit system
//                     printHex((uintptr_t)va_arg(args, void *), 16);
//                 }
//                 break;
//             case '%':
//                 printCharStr("%");
//                 break;
//             default:
//                 printCharStr("%");
//                 char_str[0] = format[i];
//                 printCharStr(char_str);
//                 break;
//             }
//         }
//         else
//         {
//             char_str[0] = format[i];
//             printCharStr(char_str);
//         }
//     }
//     va_end(args);
// }




#include "kiostream"

int cursor_x_ = 0;
int cursor_y_ = 0;
#define MAGIC_WIDTH 80
#define MAGIC_HEIGHT 25
// send a byte of data(value) to specific I/O port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// read a byte of data from specified I/O port and return
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// A very basic US QWERTY keyboard layout map.
// Only handles key presses, not releases.
static const char scancode_to_ascii[] = {
    0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
    ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

// Reads a single keypress via polling
char keyboard_input_by_polling() {
    update_cursor(cursor_x_, cursor_y_);
    uint8_t scancode;
    
    // Wait until the keyboard controller has data for us
    // Bit 0 of the status port (0x64) is set to 1 when there's data
    while ((inb(0x64) & 1) == 0) {}

    // Read the scan code from the data port
    scancode = inb(0x60);
    // printf("%d",scancode);
    // We only care about key presses (scancodes < 0x80)
    // and valid characters in our map.
    
    if (scancode < sizeof(scancode_to_ascii) && scancode_to_ascii[scancode] != 0) {
        printf("%c", scancode_to_ascii[scancode]);
        return scancode_to_ascii[scancode];
    }
    
    // Return null for unhandled keys
    return 0; 
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}
void update_cursor(int x, int y)
{
    uint16_t pos = y * 80 + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}
void disable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

#define TAB_WIDTH 4 // Standard tab width is 4 characters

void printCharStr(const char *str)
{
    unsigned short *VideoMemory = (unsigned short *)0xb8000;

    for (int i = 0; str[i] != '\0'; i++)
    {
        // Handle special control characters ---

        if (str[i] == '\n') // Newline character
        {
            cursor_y_++;
            cursor_x_ = 0;
        }
        else if (str[i] == '\b') // Backspace character
        {
            // Only move back if not at the start of the line
            if (cursor_x_ > 0)
            {
                cursor_x_--;
                // Erase the character at the new position by writing a space
                int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
                VideoMemory[offset] = (VideoMemory[offset] & 0xFF00) | ' ';
            }
        }
        else if (str[i] == '\r') // Carriage Return character
        {
            // Move cursor to the beginning of the current line
            cursor_x_ = 0;
        }
        else if (str[i] == '\t') // Tab character
        {
            // Advance cursor to the next tab stop (multiple of TAB_WIDTH)
            cursor_x_ = cursor_x_ + (TAB_WIDTH - (cursor_x_ % TAB_WIDTH));
        }
        else // normal char
        {
            int offset = cursor_y_ * MAGIC_WIDTH + cursor_x_;
            VideoMemory[offset] = (VideoMemory[offset] & 0xFF00) | str[i];
            cursor_x_++;
        }

        // If cursor goes past the right side, wrap to the next line.
        if (cursor_x_ >= MAGIC_WIDTH)
        {
            cursor_y_++;
            cursor_x_ = 0;
        }

        // If cursor goes off the bottom of the screen, scroll everything up by one line.
        if (cursor_y_ >= MAGIC_HEIGHT)
        {
            // 1. Move each line's content up by one row
            for (int y = 0; y < (MAGIC_HEIGHT - 1); y++)
            {
                for (int x = 0; x < MAGIC_WIDTH; x++)
                {
                    int current_offset = y * MAGIC_WIDTH + x;
                    int next_line_offset = (y + 1) * MAGIC_WIDTH + x;
                    VideoMemory[current_offset] = VideoMemory[next_line_offset];
                }
            }

            // 2. Clear the last line
            int last_line_offset_start = (MAGIC_HEIGHT - 1) * MAGIC_WIDTH;
            for (int x = 0; x < MAGIC_WIDTH; x++)
            {
                VideoMemory[last_line_offset_start + x] = (VideoMemory[last_line_offset_start + x] & 0xFF00) | ' ';
            }

            // 3. Reset cursor to the beginning of the now-empty last line
            cursor_y_ = MAGIC_HEIGHT - 1;
            cursor_x_ = 0;
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

// A new helper function to handle hex printing with padding
static void printHex(uintptr_t n, int digits) {
    char buffer[32]; // Buffer to hold the converted string
    
    // Convert the number to a hex string
    ullToString(n, buffer, 16, 0, 0);

    // Figure out how long the string is
    int len = 0;
    while (buffer[len] != '\0') {
        len++;
    }

    // Print the leading zeros
    for (int i = 0; i < digits - len; i++) {
        printCharStr("0");
    }

    // Print the actual number string
    printCharStr(buffer);
}



/*
 * my printf supports:
 * Specifiers: %c, %s, %d, %i, %u, %f, %x, %X, %b, %o, %p, %%
 * Flags:      '#' (alternative form for o, x, X, b)
 * Precision:  '.<number>' for %x, %X, and %f (e.g., %.8X, %.2f)
 * Length:     'l' (long), 'll' (long long)
 */
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
            
            // --- Parsing Flags, Precision, and Length ---
            int use_alternative_form = 0;
            int precision = -1; // -1 indicates not specified
            
            // 1. Flags
            if (format[i] == '#') {
                use_alternative_form = 1;
                i++;
            }

            // 2. Precision
            if (format[i] == '.') {
                i++;
                precision = 0;
                while (format[i] >= '0' && format[i] <= '9') {
                    precision = precision * 10 + (format[i] - '0');
                    i++;
                }
            }

            // 3. Length
            int is_long = 0;
            int is_long_long = 0;
            if (format[i] == 'l')
            {
                is_long = 1;
                i++;
                if (format[i] == 'l')
                {
                    is_long_long = 1;
                    is_long = 0;
                    i++;
                }
            }

            // --- Handling Specifiers ---
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
                if (is_long_long)
                    ullToString(va_arg(args, long long), buffer, 10, 1, 0);
                else if (is_long)
                    ullToString(va_arg(args, long), buffer, 10, 1, 0);
                else
                    ullToString(va_arg(args, int), buffer, 10, 1, 0);
                printCharStr(buffer);
                break;
            case 'u':
                if (is_long_long)
                    ullToString(va_arg(args, unsigned long long), buffer, 10, 0, 0);
                else if (is_long)
                    ullToString(va_arg(args, unsigned long), buffer, 10, 0, 0);
                else
                    ullToString(va_arg(args, unsigned int), buffer, 10, 0, 0);
                printCharStr(buffer);
                break;
            case 'f':
                // Pass the parsed precision. doubleToString handles the default case.
                doubleToString(va_arg(args, double), buffer, precision);
                printCharStr(buffer);
                break;
            case 'x':
            case 'X':
                {
                    unsigned long long val;
                    if (is_long_long)
                        val = va_arg(args, unsigned long long);
                    else if (is_long)
                        val = va_arg(args, unsigned long);
                    else
                        val = va_arg(args, unsigned int);

                    if (use_alternative_form && val != 0) {
                        printCharStr("0x");
                    }
                    ullToString(val, buffer, 16, 0, (format[i] == 'X'));
                    
                    int len = 0;
                    while(buffer[len] != '\0') len++;

                    // Apply padding if precision is specified
                    if (precision > len) {
                        for (int j = 0; j < precision - len; j++) {
                            printCharStr("0");
                        }
                    }

                    printCharStr(buffer);
                }
                break;
            case 'b': // note: %b and %#b are non-standard extensions
                {
                    unsigned long long val;
                    if (is_long_long)
                        val = va_arg(args, unsigned long long);
                    else if (is_long)
                        val = va_arg(args, unsigned long);
                    else
                        val = va_arg(args, unsigned int);
                    
                    if (use_alternative_form && val != 0) {
                        printCharStr("0b");
                    }
                    ullToString(val, buffer, 2, 0, 0);
                    printCharStr(buffer);
                }
                break;
            case 'o':
                {
                    unsigned long long val;
                    if (is_long_long)
                        val = va_arg(args, unsigned long long);
                    else if (is_long)
                        val = va_arg(args, unsigned long);
                    else
                        val = va_arg(args, unsigned int);
                    
                    if (use_alternative_form && val != 0) {
                        printCharStr("0");
                    }
                    ullToString(val, buffer, 8, 0, 0);
                    printCharStr(buffer);
                }
                break;
            case 'p':
                printCharStr("0x");
                if (sizeof(uintptr_t) == 4) { // 32-bit system
                    printHex((uintptr_t)va_arg(args, void *), 8);
                } else if (sizeof(uintptr_t) == 8) { // 64-bit system
                    printHex((uintptr_t)va_arg(args, void *), 16);
                }
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