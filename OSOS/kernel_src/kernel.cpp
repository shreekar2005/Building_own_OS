#include "kernel.hpp"

void task_o(void* arg) {
    (void) arg;
    for(int i=0; i<2000; i++){
        basic::printf("o");
        essential::Time::sleep(1);
    }
}

void task_X(void* arg) {
    (void) arg;
    for(int i=0; i<2000; i++){
        basic::printf("X");
        essential::Time::sleep(1);
    }
}

// NETWORK TASK
void task_Net(void* arg) {
    driver::amd_am79c973* netDriver = (driver::amd_am79c973*)arg;
    
    // Safety check
    if(netDriver == 0) {
        basic::printf("[NET] No Driver Found!\n");
        return;
    }

    uint8_t buffer[64];
    net::EtherFrameHeader* header = (net::EtherFrameHeader*)buffer;

    // Set Destination MAC: BROADCAST (FF:FF:FF:FF:FF:FF)
    for(int i=0; i<6; i++) header->dstMAC[i] = 0xFF;

    // Set Source MAC: 52:54:00:12:34:56 (QEMU Default)
    // Note: In a real stack, this comes from the driver itself.
    header->srcMAC[0] = 0x52; header->srcMAC[1] = 0x54; header->srcMAC[2] = 0x00;
    header->srcMAC[3] = 0x12; header->srcMAC[4] = 0x34; header->srcMAC[5] = 0x56;

    // Set EtherType: 0x0800 (IPv4)
    header->etherType = net::htons(0x0800);

    // Set Payload
    const char* msg = "Hello Network!";
    char* data = (char*)(buffer + sizeof(net::EtherFrameHeader));
    for(int i=0; msg[i] != 0; i++) data[i] = msg[i];

    // Loop to send packet repeatedly
    while(true) {
        basic::printf("[NET] Sending Packet...\n");
        netDriver->Send(buffer, 64);
        essential::Time::sleep(1000); // Send every 1 seconds
    }
}

void kernelTail(void* arg)
{
    KernelArgs* args = (KernelArgs*)arg;

    basic::printf("\nHELLO FROM OSOS ('help' to see commands)...\n");
    #if defined(serialMode) && serialMode == 1
        basic::printf("\nYou are using serial QEMU, Use 'ctrl+H' For Backspace\n");
    #endif
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
        // pciController.selectDrivers(&driverManager, &osos_InterruptManager);


        
        /////////////// --- MANUAL NETWORK DRIVER SETUP (Do this better by next time) --- ////////////////
        driver::amd_am79c973* netDriver = nullptr;
        
        // Check Bus 0, Device 3 (Standard QEMU Slot for NIC)
        hardware_communication::PCI_DeviceDescriptor dev = pciController.getDeviceDescriptor(0, 3, 0);
        
        if(dev.vendorId == 0x1022 && dev.deviceId == 0x2000) {
            // Found AMD Card! Instantiate manually.
            basic::printf("Found Network Card at 0:3:0\n");
            netDriver = new driver::amd_am79c973(&dev, &osos_InterruptManager);
            driverManager.addDriver(netDriver);
        } else {
            // If not found at 0:3:0, rely on auto-scan (but netDriver will be null)
            pciController.selectDrivers(&driverManager, &osos_InterruptManager);
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////




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
    essential::KThread* task2 = new essential::KThread(&task_X, nullptr);
    
    // NEW: Create Network Task (Pass the netDriver as argument)
    essential::KThread* task3 = new essential::KThread(&task_Net, (void*)netDriver);
    
    osos_ThreadManager.addThread(haltTask);
    shell.addShellTask(task1);
    shell.addShellTask(task2);
    
    if(netDriver != 0) {
        shell.addShellTask(task3);
    }
    
    // Enable Interrupts (Start the System)
    hardware_communication::InterruptManager::activate();

    while (true){asm("hlt");};
    basic::disable_cursor();
    essential::__cxa_finalize(0);
    (void)magicnumber;
}