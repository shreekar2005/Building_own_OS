#include "interrupts"


InterruptManager::InterruptManager(GDT* gdt){}
InterruptManager::~InterruptManager(){}

void InterruptManager::setInterruptDescriptorTableEntry(
    uint8_t interruptNumber,
    uint16_t codeSegmentSelectorOffset,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLever,
    uint8_t DescriptorType){

}

uintptr_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uintptr_t esp){
    printf("INTERRUPT OCCURED\n");
    return esp;
}

void InterruptManager::handleException0x00(){}

void InterruptManager::ignoreInterruptRequest(){}
void InterruptManager::handleInterruptRequest0x00(){}
void InterruptManager::handleInterruptRequest0x01(){}
