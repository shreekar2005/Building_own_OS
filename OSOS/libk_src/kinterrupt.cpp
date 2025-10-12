#include "kinterrupt"

// Global pointer to the active interrupt manager instance
// This allows our static C-style handlers to access the C++ object.
InterruptManager* activeInterruptManager;

// forward definitions of handlers (which are made .global in interruptstubs.s)
// These need to be declared extern "C" to prevent C++ name mangling,
// ensuring the linker can find the simple names defined in assembly.

IDT_row::IDT_row(){}
IDT_row::~IDT_row(){}

InterruptManager::InterruptManager(GDT* gdt):
    picMasterCommand(0x20),
    picMasterData(0x21),
    picSlaveCommand(0xA0),
    picSlaveData(0xA1){
    
    // Set the global pointer to this instance
    activeInterruptManager = this;
        
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

void InterruptManager::installTable(){
    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    idt_ptr.limit = this->limit;
    idt_ptr.base = this->base;

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}

void InterruptManager::activate(){
    __asm__ volatile ("sti");
}


uintptr_t handleInterrupt(uint8_t interruptNumber, uintptr_t esp){
    // Use the global pointer to access the PIC ports
    if (activeInterruptManager != 0) {
        activeInterruptManager->DoHandleInterrupt(interruptNumber, esp);
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
            printf("Keyboard key pressed!\n");
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
