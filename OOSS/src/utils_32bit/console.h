#ifndef CONSOLE_H
#define CONSOLE_H
#include <cstdint>
// This ensures C++ compilers see these as C functions
#ifdef __cplusplus
extern "C" {
#endif
// Public function declarations
void printCharStr(const char* str);
void clearScreen();
void printf(const char *format, ...);
char get_char();
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void update_cursor(int x, int y);
void disable_cursor();

#ifdef __cplusplus
}
#endif

#endif // CONSOLE_H