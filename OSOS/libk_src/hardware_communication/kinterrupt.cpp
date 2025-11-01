#include "hardware_communication/kinterrupt.hpp"
#include "basic/kiostream.hpp"

using namespace hardware_communication;

InterruptHandler::InterruptHandler(uint8_t interruptNumber, InterruptManager* interrupt_manager)
{
    this->interruptNumber=interruptNumber;
    this->interrupt_manager=interrupt_manager;
    interrupt_manager->handlers[interruptNumber]=this;
}

InterruptHandler::~InterruptHandler()
{
    if(interrupt_manager->handlers[interruptNumber]==this) interrupt_manager->handlers[interruptNumber]=nullptr;
}


IDT_Row::IDT_Row(){}
IDT_Row::~IDT_Row(){}

Port8BitSlow InterruptManager::picMasterCommand(0x20);
Port8BitSlow InterruptManager::picMasterData(0x21);
Port8BitSlow InterruptManager::picSlaveCommand(0xA0);
Port8BitSlow InterruptManager::picSlaveData(0xA1);


InterruptManager::InterruptManager(essential::GDT_Manager* gdt)
{
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
    setIDTEntry(0x24, kernelCSselectorOffset, &handleIRQ0x04, 0, IDT_INTERRUPT_GATE); // Serial (COM1)
}

InterruptManager::~InterruptManager(){}


void InterruptManager::setIDTEntry(
    uint8_t interruptNumber,
    uint16_t codeSegmentSelectorOffset,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLever,
    uint8_t DescriptorType)
{
    const uint8_t IDT_DESC_PRESET= 0x80;
    interruptDescriptorTable[interruptNumber].handlerAddressLowbits=(uint32_t)handler & 0xFFFF;
    interruptDescriptorTable[interruptNumber].handlerAddressHighbits=((uint32_t)handler >> 16) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].reserved=0;
    interruptDescriptorTable[interruptNumber].access=IDT_DESC_PRESET | DescriptorType | ((DescriptorPrivilegeLever & 3) << 5);
    interruptDescriptorTable[interruptNumber].kernelCodeSegmentSelector=codeSegmentSelectorOffset;
}

/// @brief Static global pointer to point currently installed interrupt manager
static InterruptManager* installed_interrupt_manager=nullptr;


void InterruptManager::installTable()
{
    installed_interrupt_manager=this;
    struct IDT_Pointer {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    IDT_Pointer idt_ptr;
    idt_ptr.limit = this->limit;
    idt_ptr.base = this->base;

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr)); // "m" will put address of `idt_ptr` in place of `%0`
    basic::printf("IDT Installed\n");
}

void InterruptManager::activate()
{
    __asm__ volatile ("sti");
    basic::printf("Interrupts Activated\n");
}

void InterruptManager::deactivate()
{
    __asm__ volatile ("cli");
    basic::printf("Interrupts Deactivated\n");
}


void InterruptManager::printLoadedTable()
{
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
    basic::printf("Entries: %d\n", (idt_ptr.limit + 1) / sizeof(IDT_Row));
    basic::printf("---\n");

    basic::printf(" Idx | Handler Address | Selector | Access Flags\n");
    IDT_Row* current_idt = (IDT_Row*)idt_ptr.base;
    uint32_t num_entries = (idt_ptr.limit + 1) / sizeof(IDT_Row);
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


void InterruptManager::printLoadedTableHeader()
{
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
    basic::printf("Entries: %d\n", (idt_ptr.limit + 1) / sizeof(IDT_Row));
    basic::printf("---\n");
}

uintptr_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uintptr_t esp)
{
    // using the global pointer "installed_interrupt_manager" to access the current interrupt manager
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