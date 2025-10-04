#ifndef CONSOLE_H
#define CONSOLE_H

// This ensures C++ compilers see these as C functions
#ifdef __cplusplus
extern "C" {
#endif

// Public function declarations
void printCharStr(const char* str);
void clearScreen();
void printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // CONSOLE_H