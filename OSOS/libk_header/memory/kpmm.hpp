#ifndef _OSOS_BASIC_KMEMORY_H
#define _OSOS_BASIC_KMEMORY_H

#include "basic/multiboot.h"
#include <cstdint>
#include <cstddef>

namespace memory
{
    // Standard page size for x86
    const uint32_t PAGE_SIZE = 4096; 
    
    // Max supported RAM (4GB). 4GB / 4KB = 1,048,576 blocks needed.
    // 1,048,576 bits / 8 bits-per-byte = 131,072 bytes bitmap size.
    const uint32_t BITMAP_SIZE = 131072; 

    void printMemoryMap(multiboot_info_t *mbi);

    class PhysicalMemoryManager {
    private:
        // The bitmap array. bit=0 means free, bit=1 means used.
        static uint8_t* memory_bitmap;
        
        // Total blocks (frames) detected in the system
        static uint32_t total_blocks;
        static uint32_t used_blocks;
        
        // Helper: Set a bit to 1 (Used)
        static void mark_used(uint32_t block_index);
        
        // Helper: Set a bit to 0 (Free)
        static void mark_free(uint32_t block_index);
        
        // Helper: Check if a bit is used
        static bool is_used(uint32_t block_index);

    public:
        // Initialize the PMM using multiboot info
        static void init(multiboot_info_t* mbi);

        // Allocates a single 4KB block. Returns physical address (or 0 if full)
        static void* allocate_block();

        // Frees a 4KB block
        static void free_block(void* address);
        
        // Get amount of free RAM (in KB)
        static uint32_t get_free_memory_kb();
    };
}

#endif