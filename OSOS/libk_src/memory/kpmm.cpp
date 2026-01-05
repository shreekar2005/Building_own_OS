#include "memory/kpmm.hpp"
#include "basic/kiostream.hpp"

// This symbol is defined by your linker script (linker.ld)
// It points to the address right after your kernel code/data.
extern "C" uint32_t end;
using namespace basic;

namespace memory
{

// Define static members
uint8_t* PhysicalMemoryManager::memory_bitmap = nullptr;
uint32_t PhysicalMemoryManager::total_blocks = 0;
uint32_t PhysicalMemoryManager::used_blocks = 0;

void printMemoryMap(multiboot_info_t *mbi)
{
    // Check if the memory map flag is set (bit 6)
    if (!(mbi->flags & (1 << 6))) {
        printf("Error: Memory map not provided by bootloader.\n");
        return;
    }

    printf("--- System Memory Map ---\n");
    
    // Use uint64_t to correctly accumulate total memory size
    uint64_t total_available_bytes = 0;
    
    // Use uintptr_t for pointer arithmetic to ensure portability
    uintptr_t mmap_start = mbi->mmap_addr;
    uintptr_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    for (multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mmap_start;
        (uintptr_t)mmap < mmap_end;
        mmap = (multiboot_memory_map_t *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size)))
    {
        const char *type_str;
        switch (mmap->type)
        {
        case MULTIBOOT_MEMORY_AVAILABLE:
            type_str = "Available";
            total_available_bytes += mmap->len;
            break;
        case MULTIBOOT_MEMORY_RESERVED:
            type_str = "Reserved";
            break;
        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            type_str = "ACPI Reclaimable";
            break;
        case MULTIBOOT_MEMORY_NVS:
            type_str = "NVS";
            break;
        case MULTIBOOT_MEMORY_BADRAM:
            type_str = "Bad RAM";
            break;
        default:
            type_str = "Unknown";
            break;
        }

        // Use %#llx for 64-bit hex values (address and length)
        printf("Addr: %#llx | Len: %.2fKB | Type: %s\n", mmap->addr, mmap->len/1024.0, type_str);
    }
    
    // Use floating point for the final calculation and display
    double total_mb = total_available_bytes / 1024.0;
    printf("Total available memory: %.2f KB\n", total_mb);
}
// -----------------------------------------------------------------------
// Helper Functions for Bit Manipulation
// -----------------------------------------------------------------------

void PhysicalMemoryManager::mark_used(uint32_t block_index) {
    uint32_t index = block_index / 8;
    uint32_t offset = block_index % 8;
    memory_bitmap[index] |= (1 << offset);
}

void PhysicalMemoryManager::mark_free(uint32_t block_index) {
    uint32_t index = block_index / 8;
    uint32_t offset = block_index % 8;
    memory_bitmap[index] &= ~(1 << offset);
}

bool PhysicalMemoryManager::is_used(uint32_t block_index) {
    uint32_t index = block_index / 8;
    uint32_t offset = block_index % 8;
    return (memory_bitmap[index] & (1 << offset)) != 0;
}

// -----------------------------------------------------------------------
// Public Interface
// -----------------------------------------------------------------------

void PhysicalMemoryManager::init(multiboot_info_t* mbi)
{
    // placing it at the address of 'end', ensuring it doesn't overwrite kernel code.
    memory_bitmap = (uint8_t*)&end;

    // Mark EVERYTHING as used (1) initially.
    // We only free what Multiboot tells us is actually RAM.
    for (int i = 0; i < (int)BITMAP_SIZE; i++) {
        memory_bitmap[i] = 0xFF; 
    }

    // Parse Multiboot Map to unlock available RAM
    uintptr_t mmap_start = mbi->mmap_addr;
    uintptr_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    for (multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mmap_start;
         (uintptr_t)mmap < mmap_end;
         mmap = (multiboot_memory_map_t *)((uintptr_t)mmap + mmap->size + sizeof(mmap->size)))
    {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // Calculate which 4KB blocks this region covers
            uint32_t start_block = mmap->addr / PAGE_SIZE;
            uint32_t num_blocks = mmap->len / PAGE_SIZE;
            
            // Calculate total blocks supported by our bitmap size
            uint32_t max_supported_blocks = BITMAP_SIZE * 8; 

            // If memory starts beyond 4GB, ignore it
            if (start_block >= max_supported_blocks) {
                continue; 
            }

            // If memory starts inside 4GB but extends beyond it, cap it.
            if (start_block + num_blocks > max_supported_blocks) {
                num_blocks = max_supported_blocks - start_block;
            }
            
            // Mark them as free
            for (uint32_t i = 0; i < num_blocks; i++) {
                mark_free(start_block + i);
            }
            
            // Track total system size roughly
            if (start_block + num_blocks > total_blocks) {
                total_blocks = start_block + num_blocks;
            }
        }
    }

    // Protect Critical Regions
    // Mark the first 4MB as USED. This covers:
    // - Real Mode IVT (0x0)
    // - BIOS Data Area
    // - Video Memory (0xB8000)
    // - Your Kernel Code (loaded at 1MB usually)
    // - GRUB Modules
    uint32_t reserved_blocks = (4 * 1024 * 1024) / PAGE_SIZE; // 1024 blocks
    for (uint32_t i = 0; i < reserved_blocks; i++) {
        mark_used(i);
    }
    
    printf("PMM Initialized. Total Blocks: %d\n", total_blocks);
}

void* PhysicalMemoryManager::allocate_block()
{
    // Iterate through bitmap to find a free bit (0)
    // Optimization: We could store the last known free index to speed this up
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!is_used(i)) {
            mark_used(i);
            used_blocks++;
            // Convert block index -> Physical Address
            return (void*)(uintptr_t)(i * PAGE_SIZE); 
        }
    }
    printf("OOM: Out of Memory!\n");
    return nullptr;
}

void PhysicalMemoryManager::free_block(void* address)
{
    uintptr_t addr = (uintptr_t)address;
    uint32_t block_index = addr / PAGE_SIZE;
    
    if (is_used(block_index)) {
        mark_free(block_index);
        used_blocks--;
    }
}

uint32_t PhysicalMemoryManager::get_free_memory_kb() {
    // This is a rough calculation based on current tracking
    // For exact numbers, you should count '0' bits in the loop
    return (total_blocks - used_blocks) * 4;
}

} // namespace memory
