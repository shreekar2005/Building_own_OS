#include "kernel.hpp"

/// @brief To pass arguments to init function
struct KernelArgs{
    multiboot_info_t *mbi;
    uint32_t magicnumber;
    essential::GDT_Manager* osos_GDT_ptr;
    essential::TaskManager* osos_TaskManager_ptr;
    hardware_communication::InterruptManager* osos_InterruptManager_ptr;
    driver::DriverManager* driverManager_ptr;
    hardware_communication::PCI_Controller* pciController_ptr;
};
/// @brief This is init task... it will run just after enabling interrupts (after setting up kernel)
void init(void* arg)
{
    (void) arg;
    multiboot_info_t *mbi = ((KernelArgs*)arg)->mbi;
    uint32_t magicnumber = ((KernelArgs*)arg)->magicnumber;
    essential::GDT_Manager* osos_GDT_ptr = ((KernelArgs*)arg)->osos_GDT_ptr;
    essential::TaskManager* osos_TaskManager_ptr = ((KernelArgs*)arg)->osos_TaskManager_ptr;
    hardware_communication::InterruptManager* osos_InterruptManager_ptr = ((KernelArgs*)arg)->osos_InterruptManager_ptr;
    driver::DriverManager* driverManager_ptr = ((KernelArgs*)arg)->driverManager_ptr;
    hardware_communication::PCI_Controller* pciController_ptr = ((KernelArgs*)arg)->pciController_ptr;

    basic::printf("\nHELLO FROM OSOS...\nEnter 'help' to see working commands :)\n");
    basic::printf("OSOS> ");

    while (true){asm("hlt");};
    basic::disable_cursor();
    essential::__cxa_finalize(0);
}

/// @brief taskA for testing multitasking
void taskA(void* arg)
{
    (void) arg;
    for(int i=0; i<100; i++){
        basic::printf("A");
        for(long long i=0; i<100000; i++){basic::printf("");} //for some short delay    
    }
}

/// @brief taskB for testing multitasking
void taskB(void* arg)
{
    (void) arg;
    for(int i=0; i<100; i++){
        basic::printf("B");
        for(long long i=0; i<100000; i++){basic::printf("");} //for some short delay
    }
}

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

    essential::TaskManager osos_TaskManager(&osos_GDT);
    

    essential::Task task1(&taskA, nullptr);
    essential::Task task2(&taskB, nullptr);
    
    // central kernel shell
    KernelShell shell(&osos_TaskManager, &task1, &task2);

    hardware_communication::InterruptManager osos_InterruptManager(&osos_GDT, &osos_TaskManager);
    
    // object of drivers so that they will handle their corresponding Interrupts
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
        pciController.selectDrivers(&driverManager, &osos_InterruptManager);
    
    driverManager.activateAll();
    
    osos_InterruptManager.installTable();
    hardware_communication::InterruptManager::printLoadedTableHeader();

    KernelArgs kernelArgs = {mbi, magicnumber, &osos_GDT, &osos_TaskManager, &osos_InterruptManager, &driverManager, &pciController};
    essential::Task initTask(&init, &kernelArgs);
    osos_TaskManager.addTask(&initTask);

    hardware_communication::InterruptManager::activate();
    
    while (true){asm("hlt");};
    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}

