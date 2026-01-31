#ifndef _OSOS_BASIC_KTYPES_HPP
#define _OSOS_BASIC_KTYPES_HPP

// -----------------------------------------------------------------------------
// Compiler-Native Type Definitions
// -----------------------------------------------------------------------------
// We use GCC/Clang built-in macros. This prevents conflicts with <cstddef> 
// and ensures 'operator new' gets the exact type signature the compiler expects.

using int8_t   = __INT8_TYPE__;
using uint8_t  = __UINT8_TYPE__;

using int16_t  = __INT16_TYPE__;
using uint16_t = __UINT16_TYPE__;

using int32_t  = __INT32_TYPE__;
using uint32_t = __UINT32_TYPE__;

using int64_t  = __INT64_TYPE__;
using uint64_t = __UINT64_TYPE__;

// -----------------------------------------------------------------------------
// Pointer & Size Types (The source of your previous error)
// -----------------------------------------------------------------------------

using intptr_t  = __INTPTR_TYPE__;
using uintptr_t = __UINTPTR_TYPE__;

using ptrdiff_t = __PTRDIFF_TYPE__;
using size_t    = __SIZE_TYPE__;

// -----------------------------------------------------------------------------
// Type Safety Checks
// -----------------------------------------------------------------------------

static_assert(sizeof(int8_t)   == 1, "int8_t must be 1 byte");
static_assert(sizeof(uint8_t)  == 1, "uint8_t must be 1 byte");
static_assert(sizeof(int16_t)  == 2, "int16_t must be 2 bytes");
static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes");
static_assert(sizeof(int32_t)  == 4, "int32_t must be 4 bytes");
static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");
static_assert(sizeof(int64_t)  == 8, "int64_t must be 8 bytes");
static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes");

static_assert(sizeof(void*) == sizeof(uintptr_t), "uintptr_t must match pointer size");

// -----------------------------------------------------------------------------
// Standard Limits (Optional but Recommended)
// -----------------------------------------------------------------------------

#define K_INT8_MIN   (-128)
#define K_INT8_MAX   (127)
#define K_UINT8_MAX  (0xff)

#define K_INT16_MIN  (-32768)
#define K_INT16_MAX  (32767)
#define K_UINT16_MAX (0xffff)

#define K_INT32_MIN  (-2147483648)
#define K_INT32_MAX  (2147483647)
#define K_UINT32_MAX (0xffffffffU)

#define K_INT64_MIN  (-9223372036854775807LL - 1)
#define K_INT64_MAX  (9223372036854775807LL)
#define K_UINT64_MAX (0xffffffffffffffffULL)

// Define NULL if it hasn't been defined by a standard header yet
#ifndef NULL
    #define NULL 0
#endif

#endif // ESSENTIAL_KTYPES_HPP