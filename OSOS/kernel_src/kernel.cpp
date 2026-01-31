#include "kernel.hpp"

using namespace basic;

void task_o(void* arg) {
    (void) arg;
    for(int i=0; i<2000; i++){
        printf("o");
        essential::Time::sleep(1);
    }
}

void task_X(void* arg) {
    (void) arg;
    for(int i=0; i<2000; i++){
        printf("X");
        essential::Time::sleep(1);
    }
}

void task_Net(void* arg) {
    driver::amd_am79c973* netDriver = (driver::amd_am79c973*)arg;
    printf("%p\n", netDriver);
    if(netDriver == 0) {
        printf("[NET ERROR] no driver found!\n");
        return;
    }

}

driver::amd_am79c973* globalNetDriver = nullptr;

void kernelTail(void* arg)
{
    KernelArgs* args = (KernelArgs*)arg;

    printf("\nHELLO FROM OSOS ('help' to see commands)...\n");
    #if defined(serialMode) && serialMode == 1
        printf("\nYou are using serial QEMU, Use 'ctrl+H' For Backspace\n");
    #endif
    printf("\nOSOS> ");
    while (true){
        args->shell->update();
        asm("hlt");
    }
    
    disable_cursor();
    essential::__cxa_finalize(0);
}

extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    essential::__callConstructors(); 
    enable_cursor(0,15);
    clearScreen();
    
    // https://wiki.osdev.org/Global_Descriptor_Table
    essential::GDT_Manager osos_GDT_Manager;
    osos_GDT_Manager.installTable();
    
    // https://wiki.osdev.org/Memory_management
    memory::PhysicalMemoryManager::init(mbi);
    memory::PagingManager::init();
    // https://wiki.osdev.org/Writing_a_memory_manager
    memory::kernel_heap.init((void*)0x1000000, 8 * 1024 * 1024);
    
    // for kernel therads
    essential::KThreadManager osos_ThreadManager(&osos_GDT_Manager);
    // for interrupts
    hardware_communication::InterruptManager osos_InterruptManager(&osos_GDT_Manager, &osos_ThreadManager);
    
    // creating KernelShell object to interract with OSOS
    KernelShell shell(&osos_ThreadManager, mbi);
    
    // https://wiki.osdev.org/Category:Drivers
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

    // activate all drivers
    driverManager.activateAll();
    osos_InterruptManager.installTable();
    
    // making Thread Arguments for kernelTail task
    KernelArgs* kArgs = new KernelArgs();
    kArgs->gdtManager = &osos_GDT_Manager;
    kArgs->interruptManager = &osos_InterruptManager;
    kArgs->driverManager = &driverManager;
    kArgs->timer = &timer;
    kArgs->shell = &shell;

    // creating tasks (or kernel therads)
    essential::KThread *haltTask= new essential::KThread(&kernelTail, kArgs);
    essential::KThread* task1 = new essential::KThread(&task_o, nullptr);
    essential::KThread* task2 = new essential::KThread(&task_X, nullptr);
    
    osos_ThreadManager.addThread(haltTask);
    shell.addShellTask(task1);
    shell.addShellTask(task2);
    
    if(globalNetDriver != nullptr) {
        essential::KThread* task3 = new essential::KThread(&task_Net, (void*)globalNetDriver);
        shell.addShellTask(task3);
        printf("[INFO] Network task added to Kernel Shell.\n");
    } else {
        printf("[WARN] Network driver not initialized.\n");
    }
    
    // Enable Interrupts (Start the System)
    hardware_communication::InterruptManager::activate();

    while (true){asm("hlt");};
    disable_cursor();
    essential::__cxa_finalize(0);
    (void)magicnumber;
}