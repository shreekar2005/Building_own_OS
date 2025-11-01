#ifndef _OSOS_ESSENTIAL_ICXXABI_H
#define _OSOS_ESSENTIAL_ICXXABI_H

#include "basic/kiostream.hpp"

#define ATEXIT_MAX_FUNCS    128
typedef unsigned uarch_t;

namespace essential
{
/// @brief Structure to hold a registered C++ ABI exit function (destructor).
/// @details Each member is at least 4 bytes large. Such that each entry is 12bytes. 128 * 12 = 1.5KB exact.
struct atexit_func_entry_t
{
    void (*destructor_func)(void *);
    void *obj_ptr;
    void *dso_handle;
};

/// @brief Executes registered destructors/cleanup functions.
/// @param f If non-null, only destructors associated with this function pointer are called. If null, all registered destructors are called in reverse order of registration.
void __cxa_finalize(void *f);

/// @brief Iterates over the list of global constructors and calls them to initialize global C++ objects.
void __callConstructors();
}

#endif