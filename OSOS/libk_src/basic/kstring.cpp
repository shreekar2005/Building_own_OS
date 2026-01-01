#include "basic/kstring.hpp"

namespace basic
{

    int strlen(const char* str)
    {
        int len = 0;
        while (str[len] != '\0')
            len++;
        return len;
    }

    int strcmp(const char* s1, const char* s2)
    {
        while (*s1 && (*s1 == *s2)) {
            s1++;
            s2++;
        }
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
    }

    int strncmp(const char* s1, const char* s2, int n)
    {
        while (n > 0 && *s1 && (*s1 == *s2)) {
            s1++;
            s2++;
            n--;
        }
        if (n == 0) return 0;
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
    }

    char* strcpy(char* dest, const char* src)
    {
        char* original_dest = dest;
        while ((*dest++ = *src++) != '\0');
        return original_dest;
    }

    char* strcat(char* dest, const char* src)
    {
        char* original_dest = dest;
        while (*dest != '\0') dest++; // Move to end of dest
        while ((*dest++ = *src++) != '\0'); // Copy src
        return original_dest;
    }

    void* memset(void* ptr, int value, int num)
    {
        unsigned char* p = (unsigned char*)ptr;
        while (num--) {
            *p++ = (unsigned char)value;
        }
        return ptr;
    }

    void* memcpy(void* dest, const void* src, int num)
    {
        char* d = (char*)dest;
        const char* s = (const char*)src;
        while (num--) {
            *d++ = *s++;
        }
        return dest;
    }
}