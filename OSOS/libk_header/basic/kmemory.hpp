#ifndef _OSOS_BASIC_KMEMORY_H
    #define _OSOS_BASIC_KMEMORY_H
    #include "basic/kiostream.hpp"
    #include "basic/multiboot.h"
    #include <cstddef>
    namespace basic{
        void __printMemoryMap(multiboot_info_t *mbi);
    }
#endif