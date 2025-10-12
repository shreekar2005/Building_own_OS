#include "kinterrupt"

// Global pointer to the active interrupt manager instance
// This allows our static C-style handlers to access the C++ object.

// forward definitions of handlers (which are made .global in interruptstubs.s)
// These need to be declared extern "C" to prevent C++ name mangling,
// ensuring the linker can find the simple names defined in assembly.

IDT_row::IDT_row(){}
IDT_row::~IDT_row(){}

Port8BitSlow InterruptManager::picMasterCommand(0x20);
Port8BitSlow InterruptManager::picMasterData(0x21);
Port8BitSlow InterruptManager::picSlaveCommand(0xA0);
Port8BitSlow InterruptManager::picSlaveData(0xA1);

InterruptManager::InterruptManager(GDT* gdt){
    picMasterCommand.write(0x11);
    picSlaveCommand.write(0x11);

    picMasterData.write(0x20); // Master PIC starts at interrupt 0x20
    picSlaveData.write(0x28); // Slave PIC starts at interrupt 0x28

    picMasterData.write(0x04); // Tell Master PIC that there is a slave at IRQ2 (00000100)
    picSlaveData.write(0x02); // Tell Slave PIC its cascade identity (00000010)

    picMasterData.write(0x01); // 8086/88 (MCS-80/85) mode
    picSlaveData.write(0x01); // 8086/88 (MCS-80/85) mode

    picMasterData.write(0x00); // Unmask all interrupts
    picSlaveData.write(0x00); // Unmask all interrupts

    base=(uintptr_t)&interruptDescriptorTable;
    limit = sizeof(interruptDescriptorTable) - 1;

    uint16_t kernelCSselectorOffset = gdt->kernel_CS_selector();
    uint8_t IDT_INTERRUPT_GATE=0xE;
    for(int i=0; i<256; i++){
        setIDTEntry(i, kernelCSselectorOffset, &ignoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    }

    // Set handlers for hardware interrupts
    setIDTEntry(0x20, kernelCSselectorOffset, &handleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE); // Timer
    setIDTEntry(0x21, kernelCSselectorOffset, &handleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE); // Keyboard
    setIDTEntry(0x2C, kernelCSselectorOffset, &handleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE); // PS/2 Mouse
}

InterruptManager::~InterruptManager(){}

void InterruptManager::setIDTEntry(
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

InterruptManager* InterruptManager::activeInterruptManager=nullptr;
void InterruptManager::installTable(){
    InterruptManager::activeInterruptManager = this;
    
    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    idt_ptr.limit = this->limit;
    idt_ptr.base = this->base;

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
    printf("IDT Installed\n");
}


uintptr_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uintptr_t esp){
    // Use the global pointer to access the PIC ports
    if (InterruptManager::activeInterruptManager!= 0) {
        InterruptManager::activeInterruptManager->DoHandleInterrupt(interruptNumber, esp);
    }
    return esp;
}

uintptr_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uintptr_t esp) {
    
    // Use a switch to handle different interrupts
    switch(interruptNumber) {
        case 0x20: // Timer
            // printf("Timer tick\n");
            break;

        case 0x21: // Keyboard
            // printf("Keyboard key pressed!\n");
            keyboard_input_by_polling();
            break;

        default:   // All other interrupts
            printf("%hd INTERRUPT OCCURRED\n", interruptNumber);
            break;
    }

    // Hardware interrupts must still be acknowledged to the PIC
    if (interruptNumber >= 0x20 && interruptNumber <= 0x2F) {
        if (interruptNumber >= 0x28) {
            picSlaveCommand.write(0x20);
        }
        picMasterCommand.write(0x20);
    }
    
    return esp;
}

void InterruptManager::activate(){
    __asm__ volatile ("sti");
    printf("Interrupts Activated\n");
}
void InterruptManager::deactivate(){
    __asm__ volatile ("cli");
    printf("Interrupts Deactivated\n");
}

void InterruptManager::printLoadedTable() {
    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    __asm__ volatile ("sidt %0" : "=m"(idt_ptr));

    printf("---\n");
    printf("INFO about : Currently Loaded IDT\n");
    printf("Base Address: %#x\n", idt_ptr.base);
    printf("Limit: %#x (%d bytes)\n", idt_ptr.limit, idt_ptr.limit);
    printf("Entries: %d\n", (idt_ptr.limit + 1) / sizeof(IDT_row));
    printf("---\n");

    printf(" Idx | Handler Address | Selector | Access Flags\n");
    IDT_row* current_idt = (IDT_row*)idt_ptr.base;
    uint32_t num_entries = (idt_ptr.limit + 1) / sizeof(IDT_row);
    for (uint32_t i = 0; i < num_entries; i++) {
        uint32_t handler_address = (current_idt[i].handlerAddressHighbits << 16) | current_idt[i].handlerAddressLowbits;

        if (handler_address != 0) {
            printf(" %3d | %#015x | %#08x | %#012x\n", 
                   i, 
                   handler_address, 
                   current_idt[i].kernelCodeSegmentSelector,
                   current_idt[i].access
            );
        }
    }
    printf("---\n");
}

void InterruptManager::printLoadedTableHeader(){

    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    __asm__ volatile ("sidt %0" : "=m"(idt_ptr));
    printf("---\n");
    printf("INFO about : Currently Loaded IDT\n");
    printf("Base Address: %#x\n", idt_ptr.base);
    printf("Limit: %#x (%d bytes)\n", idt_ptr.limit, idt_ptr.limit);
    printf("Entries: %d\n", (idt_ptr.limit + 1) / sizeof(IDT_row));
    printf("---\n");
}