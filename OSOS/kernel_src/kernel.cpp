#include <cstdint>
#include "kicxxabi"
#include "multiboot" // header for handling multiboot info (provided by grub)
#include "kiostream"  // header to use video memory : e.g. printf, keyboard_input_by_polling
#include "kmemory" // for using printMemoryMap()
#include "kgdt" // for global discripter table
#include "kinterrupt"
#include "kkeyboard"
#include "kmouse"
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

extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    __callConstructors();
    enable_cursor(0,15); // (0,15) is for blinking block
    __clearScreen();
    //-------------Global Descriptor Table -------------
    GDT osos_GDT;
    osos_GDT.installTable();
    //------------Interrupt Descriptor Table and Drivers -------------
    InterruptManager osos_InterruptManager(&osos_GDT);
    osos_InterruptManager.installTable();
    //------------Creating object of drivers so that they will handle their corresponding Interrupts------------
    MouseDriver mouse(&osos_InterruptManager);
    KeyboardDriver keyboard(&osos_InterruptManager);
    InterruptManager::activate();
    
    GDT::printLoadedTableHeader();
    InterruptManager::printLoadedTableHeader();
    printf("HELLO FROM OSOS :)\n");
    while (true){};

    disable_cursor();
    __cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}


