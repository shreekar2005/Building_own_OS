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
    push (interruptnumber)  // Arg 1: the interrupt number we stored earlier

    call  _ZN16InterruptManager15handleInterruptEhm

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
.global ignoreInterruptRequest
ignoreInterruptRequest:
    iret


// Macro for generating an IRQ stub
.macro HandleInterruptRequest num, mangled_num
.global handleInterruptRequest\mangled_num
handleInterruptRequest\mangled_num:
    movb $\num + IRQ_BASE, (interruptnumber)
    jmp common_interrupt_stub
.endm

// Generate the stubs we are using
HandleInterruptRequest 0x00, 0x00 // Timer
HandleInterruptRequest 0x01, 0x01 // Keyboard
HandleInterruptRequest 0x0C, 0x0C // PS/2 Mouse
// You can easily add more here, e.g., HandleInterruptRequest 0x0c, 0x0c for PS/2 mouse


.section .data
    interruptnumber: .byte 0
