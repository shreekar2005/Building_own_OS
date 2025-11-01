#ifndef _OSOS_BASIC_KMEMORY_H
#define _OSOS_BASIC_KMEMORY_H

#include "basic/kiostream.hpp"
#include "basic/multiboot.h"
#include <cstddef>

namespace basic
{
/// @brief Prints the system memory map provided by the Multiboot bootloader.
/// @param mbi Pointer to the Multiboot information structure.
void __printMemoryMap(multiboot_info_t *mbi);

}

#endif