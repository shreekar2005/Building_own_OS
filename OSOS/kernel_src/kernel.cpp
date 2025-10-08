#include <cstdint>
#include "./include/console"  // header to use video memory : e.g. printf
#include "./include/multiboot" // header for handling multiboot info (provided by grub)
#include "./include/memory" // for using printMemoryMap()
#include "./include/gdt" // for global discripter table

#define exC extern "C"

// using exC to avoid "name mangling" or "name decoration"
exC void kernelMain(multiboot_info_t *mbi, unsigned int magicnumber)
{
    enable_cursor(0,15); // those args will decide size or shape of cursor ((0,15) is for blinking block)
    update_cursor(0,0);

    char greeting_from_kernel[] = "Hello world! -- from OSOS kernel";
    printf("%s\nMULTIBOOT_BOOTLOADER_MAGIC : %x\n", greeting_from_kernel, magicnumber);
    printMemoryMap(mbi);
    print_gdt();
    
    while (1){
        char c = keyboard_input_by_polling();
        c=c; // just to avoid warning (unused variable)
    } // because kernel cannot stop at the end :)
    disable_cursor();
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