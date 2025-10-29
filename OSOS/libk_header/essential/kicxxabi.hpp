#ifndef _OSOS_ESSENTIAL_ICXXABI_H
#define _OSOS_ESSENTIAL_ICXXABI_H

#include "basic/kiostream.hpp"

#define ATEXIT_MAX_FUNCS    128
typedef unsigned uarch_t;

namespace essential
{
    /// @brief Structure to hold a registered C++ ABI exit function (destructor).
    struct atexit_func_entry_t
    {
        /*
        * Each member is at least 4 bytes large. Such that each entry is 12bytes.
        * 128 * 12 = 1.5KB exact.
        **/
        
        void (*destructor_func)(void *);
        void *obj_ptr;
        void *dso_handle;
    };
    void __cxa_finalize(void *f);
    void __callConstructors();
}

#endif