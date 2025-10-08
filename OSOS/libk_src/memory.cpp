#include "../kernel_src/include/memory"

void printMemoryMap(multiboot_info_t *mbi)
{
    // Check if the memory map flag is set (bit 6)
    if (!(mbi->flags & (1 << 6))) {
        printf("\nError: Memory map not provided by bootloader.\n");
        return;
    }

    printf("\n--- System Memory Map ---\n");
    
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
        printf("Addr: %#llx | Len: %fKB | Type: %s\n", mmap->addr, mmap->len/1024.0, type_str);
    }
    
    // Use floating point for the final calculation and display
    double total_mb = total_available_bytes / 1024.0;
    printf("Total available memory: %f KB\n\n", total_mb);
}