#include <cstdint>
#include "./include/multiboot" // header for handling multiboot info (provided by grub)
#include "./include/console"  // header to use  printf function

#define exC extern "C"

void printMemoryMap(multiboot_info_t *mbi);
// using exC to avoid "name mangling" or "name decoration"
exC void kernelMain(multiboot_info_t *mbi, unsigned int magicnumber)
{
    enable_cursor(0,15); // those args will decide size or shape of cursor ((0,15) is for blinking block)
    update_cursor(0,0);
    
    char greeting_from_kernel[] = "Hello world! -- from OSOS kernel";
    printf("%s\nMULTIBOOT_BOOTLOADER_MAGIC : %x\n", greeting_from_kernel, magicnumber);
    printMemoryMap(mbi);
    
    while (1){
        char c = get_char();
        c=c; // just to avoid warning (unused variable)
    } // because kernel cannot stop at the end :)
    disable_cursor();
}


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

// -------------------- Following functions will be called by loader -------------------------------

typedef void (*ctor_dtor_pointer)();

// will assign pointer values in linking stage
exC ctor_dtor_pointer start_ctors; // similar to : void (*start_ctors)();
exC ctor_dtor_pointer end_ctors;
exC ctor_dtor_pointer start_dtors; // similar to : void (*start_dtors)();
exC ctor_dtor_pointer end_dtors;

// To call constructors of global objects
exC void callConstructors()
{
    for (ctor_dtor_pointer *i = &start_ctors; i != &end_ctors; i++)
    {
        (*i)(); // calling each constructor pointer (like function pointer)
    }
}

// To call destructors of global objects
exC void callDestructors()
{
    for (ctor_dtor_pointer *i = &start_dtors; i != &end_dtors; i++)
    {
        (*i)(); // calling each constructor pointer (like function pointer)
    }
}