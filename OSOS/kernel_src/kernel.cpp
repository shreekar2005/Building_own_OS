#include "kernel.hpp"

void task_o(void* arg) {
    (void) arg;
    for(int i=0; i<2000; i++){
        basic::printf("o");
        essential::Time::sleep(1);
    }
}

void task_x(void* arg) {
    (void) arg;
    for(int i=0; i<2000; i++){
        basic::printf("x");
        essential::Time::sleep(1);
    }
}

void kernelTail(void* arg)
{
    KernelArgs* args = (KernelArgs*)arg;

    basic::printf("\nHELLO FROM OSOS (`help` to see commands)...\n");
    basic::printf("\nOSOS> ");
    while (true){
        args->shell->update();
        asm("hlt");
    }
    
    basic::disable_cursor();
    essential::__cxa_finalize(0);
}

extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    essential::__callConstructors(); 
    basic::enable_cursor(0,15);
    basic::clearScreen();
    
    // Install GDT
    essential::GDT_Manager osos_GDT_Manager;
    osos_GDT_Manager.installTable();
    
    // Initialize Memory
    memory::PhysicalMemoryManager::init(mbi);
    memory::PagingManager::init(); 
    memory::kernel_heap.init((void*)0x1000000, 8 * 1024 * 1024);
    
    // Initialize Core Kernel Managers
    essential::KThreadManager osos_ThreadManager(&osos_GDT_Manager);
    hardware_communication::InterruptManager osos_InterruptManager(&osos_GDT_Manager, &osos_ThreadManager);
    
    // Initialize Shell
    KernelShell shell(&osos_ThreadManager, mbi);
    
    // Initialize Drivers
    driver::DriverManager driverManager;
    
        driver::TimerDriver timer(&osos_InterruptManager, 1000); 
        essential::Time::setTimerDriver(&timer); // to initialize essential::Time with timer
        driverManager.addDriver(&timer);

        KeyboardEventHandler_for_kernel kbdHandler(&shell);
        driver::KeyboardDriver keyboard(&osos_InterruptManager, &kbdHandler);
        driverManager.addDriver(&keyboard);
        
        MouseEventHandler_for_kernel mouseHandler;
        driver::MouseDriver mouse(&osos_InterruptManager, &mouseHandler);
        driverManager.addDriver(&mouse);
        
        SerialEventHandler_for_kernel serialHandler(&shell);
        driver::SerialDriver serialIO(&osos_InterruptManager, &serialHandler);
        driverManager.addDriver(&serialIO);
        
        hardware_communication::PCI_Controller pciController;
        pciController.selectDrivers(&driverManager, &osos_InterruptManager);
    
    // Activate Hardware
    driverManager.activateAll();
    osos_InterruptManager.installTable();
    
    // Prepare Thread Arguments
    KernelArgs* kArgs = new KernelArgs();
    kArgs->gdtManager = &osos_GDT_Manager;
    kArgs->interruptManager = &osos_InterruptManager;
    kArgs->driverManager = &driverManager;
    kArgs->timer = &timer;
    kArgs->shell = &shell;

    // Create Tasks
    essential::KThread *haltTask= new essential::KThread(&kernelTail, kArgs);
    essential::KThread* task1 = new essential::KThread(&task_o, nullptr);
    essential::KThread* task2 = new essential::KThread(&task_x, nullptr);
    
    osos_ThreadManager.addThread(haltTask);
    shell.addShellTask(task1);
    shell.addShellTask(task2);
    
    // Enable Interrupts (Start the System)
    hardware_communication::InterruptManager::activate();

    while (true){asm("hlt");};
    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)magicnumber;
}