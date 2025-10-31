.set IRQ_BASE, 0x20

.section .text
.extern  _ZN16InterruptManager15handleInterruptEhm

// Common stub that all real interrupts will jump to
common_interrupt_stub:
    // Save all general purpose registers
    pusha

    // Save segment registers
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    // Push arguments for the C++ handler
    // C++ function signature: handleInterrupt(uint8_t interruptNumber, uintptr_t esp)
    pushl %esp              // Arg 2: current stack pointer
    push (interruptNumber)  // Arg 1: the interrupt number we stored earlier

    call  _ZN22hardware_communication16InterruptManager15handleInterruptEhm

    // The C++ handler returns the new stack pointer in EAX.
    // This cleans up the two arguments we pushed.
    movl %eax, %esp 

    // Restore segment registers
    popl %gs
    popl %fs
    popl %es
    popl %ds

    // Restore all general purpose registers
    popa

    // Return from interrupt
    iret


// A separate, simple handler for interrupts we want to ignore
.global ignoreInterrupt
ignoreInterrupt:
    iret


// Macro for generating an IRQ stub
.macro handleIRQ IRQnum, handler_suffix
.global handleIRQ\handler_suffix
handleIRQ\handler_suffix:
    movb $\IRQnum + IRQ_BASE, (interruptNumber) //interruptNumber is index for IDT table
    jmp common_interrupt_stub
.endm

// Generate the stubs we are using
// handleIRQ IRQnum, handler_suffix
handleIRQ 0x00, 0x00 // Timer : will convert its number from 0x00 to 0x20
handleIRQ 0x01, 0x01 // Keyboard : will convert its number from 0x01 to 0x21
handleIRQ 0x0C, 0x0C // PS/2 Mouse : will convert its number from 0x0C to 0x2C
handleIRQ 0x04, 0x04 // Serial (COM1) : will convert its number from 0x04 to 0x024



.section .data
    interruptNumber: .byte 0
