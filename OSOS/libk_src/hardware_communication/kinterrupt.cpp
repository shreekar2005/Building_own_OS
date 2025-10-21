#include "hardware_communication/kinterrupt"

// Global pointer to the active interrupt manager instance
// This allows our static C-style handlers to access the C++ object.

// forward definitions of handlers (which are made .global in interruptstubs.s)
// These need to be declared extern "C" to prevent C++ name mangling,
// ensuring the linker can find the simple names defined in assembly.


hardware_communication::InterruptHandler::InterruptHandler(uint8_t interruptNumber, hardware_communication::InterruptManager* interrupt_manager){
    this->interruptNumber=interruptNumber;
    this->interrupt_manager=interrupt_manager;
    interrupt_manager->handlers[interruptNumber]=this;
}
hardware_communication::InterruptHandler::~InterruptHandler(){
    if(interrupt_manager->handlers[interruptNumber]==this) interrupt_manager->handlers[interruptNumber]=nullptr;
}
// uintptr_t InterruptHandler::handleInterrupt(uintptr_t esp){
//     return esp;
// }


hardware_communication::IDT_row::IDT_row(){}
hardware_communication::IDT_row::~IDT_row(){}

hardware_communication::Port8BitSlow hardware_communication::InterruptManager::picMasterCommand(0x20);
hardware_communication::Port8BitSlow hardware_communication::InterruptManager::picMasterData(0x21);
hardware_communication::Port8BitSlow hardware_communication::InterruptManager::picSlaveCommand(0xA0);
hardware_communication::Port8BitSlow hardware_communication::InterruptManager::picSlaveData(0xA1);

hardware_communication::InterruptManager::InterruptManager(essential::GDT* gdt){
    // ICW1: Start Initialization Sequence. Both PICs are told to listen for 3 more bytes of config data.
    picMasterCommand.write(0x11);
    picSlaveCommand.write(0x11);

    // ICW2: Vector Remapping.
    // Master is told to remap its IRQs (0-7) to CPU vectors 0x20-0x27.
    picMasterData.write(0x20);
    // Slave is told to remap its IRQs (8-15) to CPU vectors 0x28-0x2F.
    picSlaveData.write(0x28);

    // ICW3: Chaining Configuration.
    // Master is told a slave is connected on its IRQ 2 line (0x04 = bit 2 set).
    picMasterData.write(0x04);
    // Slave is told its identity is 2.
    picSlaveData.write(0x02);

    // ICW4: Environment Information.
    // Both are told to operate in standard "8086/88" mode.
    picMasterData.write(0x01);
    picSlaveData.write(0x01);

    // OCW1: Interrupt Masking.
    // This is the final step, enabling all interrupts by writing a mask of all zeros.
    picMasterData.write(0x00);
    picSlaveData.write(0x00);

    base=(uintptr_t)&interruptDescriptorTable;
    limit = sizeof(interruptDescriptorTable) - 1;

    uint16_t kernelCSselectorOffset = gdt->kernel_CS_selector();
    uint8_t IDT_INTERRUPT_GATE=0xE;
    for(int i=0; i<256; i++){
        setIDTEntry(i, kernelCSselectorOffset, &ignoreInterrupt, 0, IDT_INTERRUPT_GATE);
    }

    // Set handlers for hardware interrupts
    setIDTEntry(0x20, kernelCSselectorOffset, &handleIRQ0x00, 0, IDT_INTERRUPT_GATE); // Timer
    setIDTEntry(0x21, kernelCSselectorOffset, &handleIRQ0x01, 0, IDT_INTERRUPT_GATE); // Keyboard
    setIDTEntry(0x2C, kernelCSselectorOffset, &handleIRQ0x0C, 0, IDT_INTERRUPT_GATE); // PS/2 Mouse
}

hardware_communication::InterruptManager::~InterruptManager(){}

void hardware_communication::InterruptManager::setIDTEntry(
    uint8_t interruptNumber,
    uint16_t codeSegmentSelectorOffset,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLever,
    uint8_t DescriptorType){
        const uint8_t IDT_DESC_PRESET= 0x80;
        interruptDescriptorTable[interruptNumber].handlerAddressLowbits=(uint32_t)handler & 0xFFFF;
        interruptDescriptorTable[interruptNumber].handlerAddressHighbits=((uint32_t)handler >> 16) & 0xFFFF;
        interruptDescriptorTable[interruptNumber].reserved=0;
        interruptDescriptorTable[interruptNumber].access=IDT_DESC_PRESET | DescriptorType | ((DescriptorPrivilegeLever & 3) << 5);
        interruptDescriptorTable[interruptNumber].kernelCodeSegmentSelector=codeSegmentSelectorOffset;
}

static hardware_communication::InterruptManager* installed_interrupt_manager=nullptr;
void hardware_communication::InterruptManager::installTable(){
    installed_interrupt_manager=this;
    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    idt_ptr.limit = this->limit;
    idt_ptr.base = this->base;

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
    basic::printf("IDT Installed\n");
}



void hardware_communication::InterruptManager::activate(){
    __asm__ volatile ("sti");
    basic::printf("Interrupts Activated\n");
}
void hardware_communication::InterruptManager::deactivate(){
    __asm__ volatile ("cli");
    basic::printf("Interrupts Deactivated\n");
}

void hardware_communication::InterruptManager::printLoadedTable() {
    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    __asm__ volatile ("sidt %0" : "=m"(idt_ptr));

    basic::printf("---\n");
    basic::printf("INFO about : Currently Loaded IDT\n");
    basic::printf("Base Address: %#x\n", idt_ptr.base);
    basic::printf("Limit: %#x (%d bytes)\n", idt_ptr.limit, idt_ptr.limit);
    basic::printf("Entries: %d\n", (idt_ptr.limit + 1) / sizeof(hardware_communication::IDT_row));
    basic::printf("---\n");

    basic::printf(" Idx | Handler Address | Selector | Access Flags\n");
    hardware_communication::IDT_row* current_idt = (hardware_communication::IDT_row*)idt_ptr.base;
    uint32_t num_entries = (idt_ptr.limit + 1) / sizeof(hardware_communication::IDT_row);
    for (uint32_t i = 0; i < num_entries; i++) {
        uint32_t handler_address = (current_idt[i].handlerAddressHighbits << 16) | current_idt[i].handlerAddressLowbits;

        if (handler_address != 0) {
            basic::printf(" %3d | %#015x | %#08x | %#012x\n", 
                   i, 
                   handler_address, 
                   current_idt[i].kernelCodeSegmentSelector,
                   current_idt[i].access
            );
        }
    }
    basic::printf("---\n");
}

void hardware_communication::InterruptManager::printLoadedTableHeader(){

    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    __asm__ volatile ("sidt %0" : "=m"(idt_ptr));
    basic::printf("---\n");
    basic::printf("INFO about : Currently Loaded IDT\n");
    basic::printf("Base Address: %#x\n", idt_ptr.base);
    basic::printf("Limit: %#x (%d bytes)\n", idt_ptr.limit, idt_ptr.limit);
    basic::printf("Entries: %d\n", (idt_ptr.limit + 1) / sizeof(hardware_communication::IDT_row));
    basic::printf("---\n");
}


uintptr_t hardware_communication::InterruptManager::handleInterrupt(uint8_t interruptNumber, uintptr_t esp){
    // Use the global pointer "installed_interrupt_manager" to access the current interrupt manager
    if(installed_interrupt_manager->handlers[interruptNumber]!=nullptr){
        esp = installed_interrupt_manager->handlers[interruptNumber]->handleInterrupt(esp);
    }

    else if(interruptNumber!=0x20){ //0x20 is Hardware Timer Interrupt
        basic::printf("UNHANDLED INTERRUPT %#hx\n",interruptNumber);
    }

    // Hardware interrupts must still be acknowledged to the PIC
    if (0x20 <=interruptNumber && interruptNumber <= 0x2F) {
        picMasterCommand.write(0x20);
        if (0x28 <= interruptNumber) picSlaveCommand.write(0x20);
    }
    
    return esp;
}