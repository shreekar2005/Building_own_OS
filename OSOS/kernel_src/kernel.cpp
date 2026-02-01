#include "kernel.hpp"

using namespace basic;


KernelArgs* kArgs;

void task_o(void* arg) {
    (void) arg;
    int time1, time2;
    time1 = kArgs->timer->getUptimeMS();
    for(int i=0; i<100; i++){
        printf("o");
        essential::Time::sleep(40);
    }
    time2 = kArgs->timer->getUptimeMS();
    printf("\ntask1 took : %dms\n",time2-time1);
}

void task_X(void* arg) {
    (void) arg;
    int time1, time2;
    time1 = kArgs->timer->getUptimeMS();
    for(int i=0; i<100; i++){
        if(i%1==0) printf("X");
        essential::Time::sleep(40);
    }
    time2 = kArgs->timer->getUptimeMS();
    printf("\ntask2 took : %dms\n",time2-time1);
}

void task_Net(void* arg) {
    driver::amd_am79c973* netDriver = (driver::amd_am79c973*)arg;
    printf("%p\n", netDriver);
    if(netDriver == 0) {
        printf("[NET ERROR] no driver found!\n");
        return;
    }
    return;

}

driver::amd_am79c973* globalNetDriver = nullptr;

void kernelTail(void* arg)
{
    KernelArgs* kArgs = (KernelArgs*)arg;

    printf("\nHELLO FROM OSOS ('help' to see commands)...\n");
    
    #if defined(serialMode) && serialMode == 1
    printf("\nYou are using serial QEMU, Use 'ctrl+H' For Backspace\n");
    #endif

    printf("\nOSOS> ");
    while (true){
        kArgs->shell->update();
        asm("hlt");
    }
    
    disable_cursor();
    essential::__cxa_finalize(0);
}

// https://wiki.osdev.org/Global_Descriptor_Table
essential::GDT_Manager osos_GDT_Manager; // making this as global variable, so that it will not use stack and it last forever

extern "C" void kernelMain(multiboot_info_t *mbi, uint32_t magicnumber)
{
    essential::__callConstructors(); 
    enable_cursor(0,15);
    clearScreen(); printf("\n");

    osos_GDT_Manager.installTable();
    
    // https://wiki.osdev.org/Memory_management
    memory::PhysicalMemoryManager::init(mbi);

    memory::PagingManager::init();
    // https://wiki.osdev.org/Writing_a_memory_manager
    memory::kernel_heap.init((void*)0x1000000, 8 * 1024 * 1024);
    
    // for kernel therads
    concurrency::KThreadManager* osos_ThreadManager = new concurrency::KThreadManager(&osos_GDT_Manager);
    // for interrupts
    hardware_communication::InterruptManager* osos_InterruptManager = new hardware_communication::InterruptManager(&osos_GDT_Manager, osos_ThreadManager);
    
    // creating KernelShell object to interract with OSOS
    KernelShell* shell = new KernelShell(osos_ThreadManager, mbi);
    
    // https://wiki.osdev.org/Category:Drivers
    driver::DriverManager* driverManager = new driver::DriverManager();
    
        driver::TimerDriver* timer = new driver::TimerDriver(osos_InterruptManager, 1000); 
        essential::Time::setTimerDriver(timer); // to initialize essential::Time with timer
        driverManager->addDriver(timer);

        KeyboardEventHandler_for_kernel* kbdHandler = new KeyboardEventHandler_for_kernel(shell);
        driver::KeyboardDriver* keyboard = new driver::KeyboardDriver(osos_InterruptManager, kbdHandler);
        driverManager->addDriver(keyboard);
        
        MouseEventHandler_for_kernel* mouseHandler = new MouseEventHandler_for_kernel();
        driver::MouseDriver* mouse = new driver::MouseDriver(osos_InterruptManager, mouseHandler);
        driverManager->addDriver(mouse);
        
        SerialEventHandler_for_kernel* serialHandler = new SerialEventHandler_for_kernel(shell);
        driver::SerialDriver* serialIO = new driver::SerialDriver(osos_InterruptManager, serialHandler);
        driverManager->addDriver(serialIO);

        hardware_communication::PCI_Controller* pciController = new hardware_communication::PCI_Controller();
        pciController->selectDrivers(driverManager, osos_InterruptManager);

    // activate all drivers
    driverManager->activateAll();
    osos_InterruptManager->installTable();
    
    // making Thread Arguments for kernelTail task
    kArgs = new KernelArgs();
    kArgs->gdtManager = &osos_GDT_Manager;
    kArgs->interruptManager = osos_InterruptManager;
    kArgs->driverManager = driverManager;
    kArgs->timer = timer;
    kArgs->shell = shell;
    kArgs->theradManager = osos_ThreadManager;

    // creating tasks (or kernel therads)
    concurrency::KThread* task1 = new concurrency::KThread(&task_o, nullptr);
    concurrency::KThread* task2 = new concurrency::KThread(&task_X, nullptr);
    
    
    shell->addShellTask(task1);
    shell->addShellTask(task2);
    
    if(globalNetDriver != nullptr) {
        concurrency::KThread* task3 = new concurrency::KThread(&task_Net, (void*)globalNetDriver);
        shell->addShellTask(task3);
        printf("[INFO] Network task added to Kernel Shell.\n");
    } else {
        printf("[WARN] Network driver not initialized.\n");
    }
    
    concurrency::KThread *haltTask= new concurrency::KThread(&kernelTail, kArgs);
    haltTask->start(); // will not start executing till interrupts are off

    // Enable Interrupts (Start the System)
    hardware_communication::InterruptManager::activate();

    while (true){asm("hlt");};
    disable_cursor();
    essential::__cxa_finalize(0);
    (void)magicnumber;
}