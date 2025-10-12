#include "kicxxabi"
#include "multiboot" // header for handling multiboot info (provided by grub)
#include "kiostream"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "kmemory" // for using printMemoryMap()
#include "kgdt" // for global discripter table
#include "kinterrupt"

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
    __callConstructors();
    enable_cursor(0,15); // (0,15) is for blinking block
    __clearScreen();

    GDT OSOS_GDT;
    OSOS_GDT.installTable();
    
    GDT::printLoadedTableHeader();
    InterruptManager OSOS_IDT(&OSOS_GDT);
    OSOS_IDT.installTable();
    InterruptManager::activate();
    InterruptManager::printLoadedTableHeader();
    
    printf("Hello from OSOS kernel\n");
    while (true){
        // printf("ThisIsLongWord");
    }
    disable_cursor();
    __cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}


