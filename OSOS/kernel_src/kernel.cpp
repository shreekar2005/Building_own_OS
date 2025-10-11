#include "kicxxabi"
#include "multiboot" // header for handling multiboot info (provided by grub)
#include "kiostream"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "kmemory" // for using printMemoryMap()
#include "kgdt" // for global discripter table

class TestClass{
    public :
        TestClass(){
            printf("TestClass constructor called\n");
        }
        ~TestClass(){
            printf("TestClass destructor called\n");
        }
};
TestClass a_global_instance;

extern "C" void kernelMain(multiboot_info_t *mbi, unsigned int magicnumber)
{
    GDT OSOS_GDT;
    OSOS_GDT.installTable();
    __callConstructors();
    enable_cursor(0,15); // those args will decide size or shape of cursor ((0,15) is for blinking block)
    
    
    
    char greeting_from_kernel[] = "Hello world! -- from OSOS kernel";
    printf("%s\nMULTIBOOT_BOOTLOADER_MAGIC : %x\n", greeting_from_kernel, magicnumber);\
    
    // GDT::printTable();
    // __printMemoryMap(mbi);
    
    while (1){
        char c = keyboard_input_by_polling();
        (void)c; // just to avoid warning (unused variable)
    }
    disable_cursor();
    __cxa_finalize(0);
    (void)mbi;
}


