#ifndef _OSOS_ESSENTIAL_KSTDARG_HPP
#define _OSOS_ESSENTIAL_KSTDARG_HPP

// -----------------------------------------------------------------------------
// Variadic Argument Primitives
// -----------------------------------------------------------------------------
// These rely on compiler built-ins. This is the only portable way to handle
// variadic arguments (like in kprintf) across different architectures (x86, ARM, etc).

// The type used to iterate arguments
using va_list = __builtin_va_list;

// Initialize the list. 
// 'v' is the va_list instance. 
// 'l' is the last named argument of the function before the '...'.
#define va_start(v, l)  __builtin_va_start(v, l)

// Cleanup the list when done.
#define va_end(v)       __builtin_va_end(v)

// Retrieve the next argument.
// 'v' is the va_list instance.
// 't' is the TYPE you expect to read next (e.g., int, char*, size_t).
#define va_arg(v, t)    __builtin_va_arg(v, t)

// Copy a list (useful if you need to traverse the args twice).
#define va_copy(d, s)   __builtin_va_copy(d, s)

#endif // ESSENTIAL_KSTDARG_HPP