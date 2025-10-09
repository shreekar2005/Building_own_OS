#include <cstdint>
#include "multiboot" // header for handling multiboot info (provided by grub)
#include "kiostream"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "kmemory" // for using printMemoryMap()
#include "kgdt" // for global discripter table

// using exC to avoid "name mangling" or "name decoration"
extern "C" void kernelMain(multiboot_info_t *mbi, unsigned int magicnumber)
{
    enable_cursor(0,15); // those args will decide size or shape of cursor ((0,15) is for blinking block)
    update_cursor(0,0);

    char greeting_from_kernel[] = "Hello world! -- from OSOS kernel";
    printf("%s\nMULTIBOOT_BOOTLOADER_MAGIC : %x\n", greeting_from_kernel, magicnumber);
    // printMemoryMap(mbi);
    init_GDT();
    print_GDT();
    
    while (1){
        char c = keyboard_input_by_polling();
        c=c; // just to avoid warning (unused variable)
    }
    disable_cursor();
    mbi=mbi;
}


// -------------- Following functions will be called by loader ------------------

// To call constructors of global objects
extern "C" void (*ctors_start)();
extern "C" void (*ctors_end)();
extern "C" void callConstructors(){
    for (void (**p)() = &ctors_start; p < &ctors_end; ++p)
    {
        (*p)(); // Call the constructor
    }
}

// To call destructors of global objects
extern "C" void(*dtors_start)();
extern "C" void(*dtors_end)();
extern "C" void callDestructors(){
    for (void (**p)() = &dtors_start; p < &dtors_end; ++p)
    {
        (*p)(); // Call the destructor
    }
}