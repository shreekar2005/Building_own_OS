#ifndef _OSOS_HARDWARECOMMUNCATION_KINTERRUPT_H
#define _OSOS_HARDWARECOMMUNCATION_KINTERRUPT_H

#include <cstdint>
#include "essential/kgdt.hpp"
#include "hardware_communication/kport.hpp"

extern "C" 
{
    // following functions are defined in kinterruptstub.s, From where we are calling InterruptManager::handleInterrupt with interruptNumber = IRQ+20 and current esp

    void ignoreInterrupt(); // Ignore Interrupt
    void handleIRQ0x00();   // Timer
    void handleIRQ0x01();   // Keyboard
    void handleIRQ0x0C();   // PS/2 Mouse 
    void handleIRQ0x04();   // Serial IO (COM1)
}

namespace hardware_communication
{
class InterruptManager; //making forward definition for this class (needed in InterruptHandler)

/// @brief Base class (interface) for handling specific CPU interrupts.
class InterruptHandler{
    protected:
        uint8_t interruptNumber;
        InterruptManager* interrupt_manager;
        InterruptHandler(uint8_t interruptNumber, InterruptManager* interrupt_manager);
        ~InterruptHandler();
    public:
        virtual uintptr_t handleInterrupt(uintptr_t esp)=0;
};

/// @brief Represents a single 8-byte entry (gate descriptor) in the Interrupt Descriptor Table (IDT).
class IDT_Row{
    private:
        uint16_t handlerAddressLowbits;
        uint16_t kernelCodeSegmentSelector;
        uint8_t reserved;
        uint8_t access;
        uint16_t handlerAddressHighbits;
    public:
        IDT_Row();
        ~IDT_Row();
        friend class InterruptManager;
}__attribute__((packed));

/// @brief Manages the Interrupt Descriptor Table (IDT) and Programmable Interrupt Controllers (PICs), dispatching interrupts to registered handlers.
class InterruptManager{
    
    friend class InterruptHandler;

    public :
        uint16_t limit;
        uint32_t base;
    protected:
        IDT_Row interruptDescriptorTable[256];
        InterruptHandler* handlers[256];

        /// @brief Populates a specific entry in the Interrupt Descriptor Table (IDT).
        /// @param interruptNumber The index of the IDT entry to set (0-255).
        /// @param codeSegmentSelectorOffset The segment selector for the interrupt handler code.
        /// @param handler A function pointer to the assembly stub that handles the interrupt.
        /// @param DescriptorPrivilegeLever The DPL (Descriptor Privilege Level) for the interrupt gate.
        /// @param DescriptorType The type of the descriptor (e.g., 0xE for 32-bit Interrupt Gate).
        void setIDTEntry(
            uint8_t interruptNumber,
            uint16_t codeSegmentSelectorOffset,
            void (*handler)(),
            uint8_t DescriptorPrivilegeLever,
            uint8_t DescriptorType);

        static hardware_communication::Port8BitSlow picMasterCommand;
        static hardware_communication::Port8BitSlow picMasterData;
        static hardware_communication::Port8BitSlow picSlaveCommand;
        static hardware_communication::Port8BitSlow picSlaveData;

    public:
        InterruptManager(essential::GDT_Manager* gdt);
        ~InterruptManager();

        /// @brief Loads the Interrupt Descriptor Table (IDT) into the CPU's IDTR register.
        void installTable();

        /// @brief Enables interrupts globally by executing the 'sti' instruction.
        static void activate();

        /// @brief Disables interrupts globally by executing the 'cli' instruction.
        static void deactivate();

        /// @brief Prints the details of all entries in the currently loaded IDT.
        static void printLoadedTable();

        /// @brief Prints the header information (base, limit, count) of the currently loaded IDT.
        static void printLoadedTableHeader();

        /// @brief The central interrupt dispatching function called by assembly stubs.
        /// @param interruptNumber The number of the interrupt that occurred.
        /// @param esp The stack pointer (Extended Stack Pointer) from the context of the interrupted process.
        /// @return The updated stack pointer, typically the same as the input unless the handler modified the stack frame.
        static uintptr_t handleInterrupt(uint8_t interruptNumber, uintptr_t esp);
};
}

#endif