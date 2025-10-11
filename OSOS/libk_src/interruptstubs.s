.set IRQ_BASE, 0x20

.section .text
.extern _ZN16InterruptManager15handleInterruptEhm

.macro HandleExceptionRequest num // I am not sure about this (because if i write num=0x05 then there will be 19 instead of 16)
.global _ZN16InterruptManager19handleException\num\()Ev // why global? why not extern?
    movb %\num, (interruptnumber)
    jmp int_bottom
.endm


.macro HandleInterruptRequest num
.global _ZN16InterruptManager26handleInterruptRequest\num\()Ev // why global? why not extern?
    movb %\num + IRQ_BASE, (interruptnumber)
    jmp int_bottom
.endm

HandleInterruptRequest 0x00
HandleInterruptRequest 0x01

int_bottom:

    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    pushl %esp
    push (interruptnumber)
    call _ZN16InterruptManager15handleInterruptEhm
    movl %eax, %esp //similar to addl $5, %esp because we are returning value of esp in "InterruptManager::handleinterrupt" function

    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa

    iret

.section .data
    interruptnumber .byte 0