#include "kernel.hpp"

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

essential::Task task1(&taskA, nullptr);
essential::Task task2(&taskB, nullptr);

/// @brief This is halt for kernelMain
void halt(void* arg)
{
    basic::printf("\nHELLO FROM OSOS (`help` to see commands)...\npress Enter to continue... :)");
    (void) arg;
    while (true){asm("hlt");};
    basic::disable_cursor();
    essential::__cxa_finalize(0);
}

/// @brief The main entry point for the C++ kernel.
/// @param mbi Pointer to the Multiboot information structure provided by GRUB.
/// @param magicnumber The magic number passed by GRUB to verify boot.
extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    essential::__callConstructors(); 
    basic::enable_cursor(0,15);
    basic::clearScreen();
    
    // initiating PMM
    memory::PhysicalMemoryManager::init(mbi);
    // initiating Paging
    memory::PagingManager::init(); // Maps 0-24MB now
    // initiating Heap : Start at 16MB (0x1000000), Size 8MB
    memory::kernel_heap.init((void*)0x1000000, 8 * 1024 * 1024);

    // testing C++ new
    int* arr = new int[10];
    arr[0] = 99;
    basic::printf("Heap Test (new): %d\n", arr[0]);
    delete[] arr;

    // testing C malloc
    void* ptr = malloc(100);
    basic::printf("Heap Test (malloc): 0x%x\n", ptr);
    free(ptr);

    essential::GDT_Manager osos_GDT_Manager;
    osos_GDT_Manager.installTable();
    essential::GDT_Manager::printLoadedTableHeader();

    essential::TaskManager osos_TaskManager(&osos_GDT_Manager);

    // central kernel shell
    KernelShell shell(&osos_TaskManager, mbi);

    hardware_communication::InterruptManager osos_InterruptManager(&osos_GDT_Manager, &osos_TaskManager);
    
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

    essential::Task haltTask(&halt, nullptr);
    osos_TaskManager.addTask(&haltTask);

    shell.addShellTask(&task1);
    shell.addShellTask(&task2);
    
    hardware_communication::InterruptManager::activate();
    while (true){asm("hlt");};
    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)mbi;
    (void)magicnumber;
}

