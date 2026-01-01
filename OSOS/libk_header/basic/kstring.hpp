#ifndef _OSOS_BASIC_KSTRING_H
#define _OSOS_BASIC_KSTRING_H

#include <cstdint>

namespace basic
{
    /// @brief Calculates the length of a null-terminated string.
    /// @param str The input string.
    /// @return The number of characters preceding the null byte.
    int strlen(const char* str);

    /// @brief Compares two strings lexicographically.
    /// @param s1 The first string.
    /// @param s2 The second string.
    /// @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2.
    int strcmp(const char* s1, const char* s2);

    /// @brief Compares up to 'n' characters of two strings.
    /// @param s1 The first string.
    /// @param s2 The second string.
    /// @param n The maximum number of characters to compare.
    /// @return 0 if equal (or n reached), <0 if s1 < s2, >0 if s1 > s2.
    int strncmp(const char* s1, const char* s2, int n);

    /// @brief Copies the source string to the destination buffer.
    /// @param dest The buffer to copy into.
    /// @param src The string to copy from.
    /// @return A pointer to the destination string.
    char* strcpy(char* dest, const char* src);

    /// @brief Appends the source string to the end of the destination string.
    /// @param dest The string to append to.
    /// @param src The string to add.
    /// @return A pointer to the destination string.
    char* strcat(char* dest, const char* src);

    /// @brief Fills a block of memory with a specific value.
    /// @param ptr Pointer to the block of memory to fill.
    /// @param value The value to be set.
    /// @param num Number of bytes to be set to the value.
    /// @return A pointer to the block of memory.
    void* memset(void* ptr, int value, int num);

    /// @brief Copies 'num' bytes from source memory to destination memory.
    /// @param dest Pointer to the destination memory.
    /// @param src Pointer to the source memory.
    /// @param num Number of bytes to copy.
    /// @return A pointer to the destination.
    void* memcpy(void* dest, const void* src, int num);
}

#endif