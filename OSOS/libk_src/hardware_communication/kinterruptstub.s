.set IRQ_BASE, 0x20

.section .text
.extern  _ZN22hardware_communication16InterruptManager15handleInterruptEhm

common_interrupt_stub:
    // follow register order like defined in CPUStatus (defined in kmultitasking.hpp)
    pushl $0 // Dummy error code

    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax

    // void handleInterrupt(uint8_t interruptNumber, uintptr_t esp)
    
    pushl %esp // Arg 2: Current Stack Pointer (points to CPUState)
    
    xorl %eax, %eax // Clear EAX
    movb (interruptNumber), %al
    pushl %eax // Arg 1: The Interrupt Number (from global var)

    // Call the C++ Method
    call  _ZN22hardware_communication16InterruptManager15handleInterruptEhm

    // The C++ handler returns the (possibly new) stack pointer in EAX.
    movl %eax, %esp 

    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp

    popl %gs
    popl %fs
    popl %es
    popl %ds

    add $4, %esp
    iret

.global ignoreInterrupt
ignoreInterrupt:
    iret


// Macro for Hardware Interrupts (IRQs)
.macro handleIRQ IRQnum, handler_suffix
.global handleIRQ\handler_suffix
handleIRQ\handler_suffix:
    movb $\IRQnum + IRQ_BASE, (interruptNumber)
    jmp common_interrupt_stub
.endm


// Generate the stubs
handleIRQ 0x00, 0x00 // Timer (IRQ 0) -> Int 0x20
handleIRQ 0x01, 0x01 // Keyboard (IRQ 1) -> Int 0x21
handleIRQ 0x0C, 0x0C // Mouse (IRQ 12) -> Int 0x2C
handleIRQ 0x04, 0x04 // COM1 (IRQ 4) -> Int 0x24


.section .data
    interruptNumber: .byte 0