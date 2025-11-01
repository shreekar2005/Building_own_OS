#include "kernel.hpp"

/// @brief The main entry point for the C++ kernel.
/// @param mbi Pointer to the Multiboot information structure provided by GRUB.
/// @param magicnumber The magic number passed by GRUB to verify boot.
extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    essential::__callConstructors(); 
    basic::enable_cursor(0,15);
    basic::clearScreen();

    essential::GDT_Manager osos_GDT;
    osos_GDT.installTable();
    essential::GDT_Manager::printLoadedTableHeader();

    // Creating the central shell
    KernelShell shell;

    
    hardware_communication::InterruptManager osos_InterruptManager(&osos_GDT);
    
    //Creating object of drivers so that they will handle their corresponding Interrupts
    driver::DriverManager driverManager;
    
        KeyboardEventHandler_for_kernel keyboardEventHandler_for_kernel(&shell);
        driver::KeyboardDriver keyboard(&osos_InterruptManager,&keyboardEventHandler_for_kernel);
        driverManager.addDriver(&keyboard);
        
        MouseEventHandler_for_kernel mouseEventHandler_for_kernel;
        driver::MouseDriver mouse(&osos_InterruptManager, &mouseEventHandler_for_kernel);
        driverManager.addDriver(&mouse);
        
        SerialEventHandler_for_kernel serialEventHandler_for_kernel(&shell);
        driver::SerialDriver serialIO(&osos_InterruptManager, &serialEventHandler_for_kernel);
        driverManager.addDriver(&serialIO);
        
        hardware_communication::PCI_Controller pciController;
        pciController.selectDrivers(&driverManager);
    
    driverManager.activateAll();
    
    osos_InterruptManager.installTable();
    hardware_communication::InterruptManager::printLoadedTableHeader();
    hardware_communication::InterruptManager::activate();
    
    basic::printf("HELLO FROM OSOS :)\n");
    basic::printf("OSOS> ");
    while (true){asm("hlt");};
    
    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}

