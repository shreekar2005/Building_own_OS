#include "basic/kmemory.hpp"

/// @brief Prints the system memory map provided by the Multiboot bootloader.
/// @param mbi Pointer to the Multiboot information structure.
void basic::__printMemoryMap(multiboot_info_t *mbi)
{
    // Check if the memory map flag is set (bit 6)
    if (!(mbi->flags & (1 << 6))) {
        basic::printf("\nError: Memory map not provided by bootloader.\n");
        return;
    }

    basic::printf("\n--- System Memory Map ---\n");
    
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
        basic::printf("Addr: %#llx | Len: %.2fKB | Type: %s\n", mmap->addr, mmap->len/1024.0, type_str);
    }
    
    // Use floating point for the final calculation and display
    double total_mb = total_available_bytes / 1024.0;
    basic::printf("Total available memory: %.2f KB\n\n", total_mb);
}


/**
 * The compiler requires these functions to be defined to handle memory
 * allocation and deallocation, especially for classes with virtual
 * destructors. Since we are not linking the standard library, we must
 * provide our own. For now, they don't have to do anything.
 */

/// @brief Overload of the 'new' operator (stub).
/// @param size The size of memory to allocate.
/// @return Always returns nullptr as this is a stub.
void* operator new(size_t size) noexcept
{
    (void)size;
    return nullptr;
}

/// @brief Overload of the 'new[]' operator (stub).
/// @param size The size of memory to allocate for the array.
/// @return Always returns nullptr as this is a stub.
void* operator new[](size_t size) noexcept
{
    (void)size;
    return nullptr;
}

/// @brief Overload of the 'delete' operator (stub).
/// @param ptr Pointer to the memory to deallocate.
void operator delete(void* ptr) noexcept
{
    (void)ptr;
}

/// @brief Overload of the 'delete' operator with size (stub).
/// @param ptr Pointer to the memory to deallocate.
/// @param size The size of the memory block.
void operator delete(void* ptr, size_t size) noexcept
{
    (void)ptr;
    (void)size;
}

/// @brief Overload of the 'delete[]' operator (stub).
/// @param ptr Pointer to the array memory to deallocate.
void operator delete[](void* ptr) noexcept
{
    (void)ptr;
}

/// @brief Overload of the 'delete[]' operator with size (stub).
/// @param ptr Pointer to the array memory to deallocate.
/// @param size The size of the memory block.
void operator delete[](void* ptr, size_t size) noexcept
{
    (void)ptr;
    (void)size;
}