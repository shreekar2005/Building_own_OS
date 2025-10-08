#include "../kernel_src/include/memory"

void printMemoryMap(multiboot_info_t *mbi)
{
    // First, check if the memory map flag is set in the multiboot info struct.
    // The flag for the memory map is bit 6.
    if (!(mbi->flags & (1 << 6)))
    {
        printf("\nError: Memory map not provided by bootloader.\n");
        return;
    }

    printf("\n--- System Memory Map ---\n");
    float totalMemory=0;
    // Loop through all the memory map entries
    for (
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
        (unsigned int)mmap < mbi->mmap_addr + mbi->mmap_length;
        // The way to get the next entry is to add the size of the current entry
        // to the current entry's pointer.
        mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(mmap->size)))
    {
        const char *type_str;
        switch (mmap->type)
        {
        case MULTIBOOT_MEMORY_AVAILABLE:
            type_str = "Available";
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
        // We use %x for address
        printf("Addr: %llx | Len: %lldB | Type: %s\n", mmap->addr, mmap->len, type_str);
        totalMemory+=(unsigned int)mmap->len;
    }
    totalMemory/=1024*1024;
    printf("total memory = %fMB\n\n",totalMemory);
}