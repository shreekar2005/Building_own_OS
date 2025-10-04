# actually this loader is part of kernel only, but we need some low level instructions to actually get into highlevel (c++) kernel. So i am writing this loader program
.set MAGIC, 0x1badb002 
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC+FLAGS)
# .set will actully set that variable to given value only for assembler (variable will be not stored in object file)

.section .multiboot # here it is start of file
    .long MAGIC # first 32bits of the loader file should be this magic number
    .long FLAGS # this is required to get information like memory map from bootloader (like GRUB) to our kerlen
    .long CHECKSUM

# multiboot structure (not above .multiboot) stores some information e.g. size of RAM etc. (boot loder creates this multiboot structure and stores pointer to that struction in EBX register)
# with multiboot structure pointer, bootloader also copies magic number in EAX register
# so we are going to give first param EBX and second param EAX to our kernelMain function

# extern From kernel.cpp
.extern callConstructors
.extern callDestructors
.extern kernelMain
.extern clearScreen
.extern printf

.section .text # now actual code begins
    .global loader
    loader:
        mov $kernel_stack, %esp # move kernel_stack value into esp register 
        call clearScreen

        call callConstructors
        push %eax # the magic number is stored in EAX.
        push %ebx # bootloader provides an information structure when the kernel boots. On x86, a physical pointer is stored in EBX
        call kernelMain # this not suppose to come again from kernelMain
        add $8, %esp
        call callDestructors

        push $myString
        call printf
        add $4, %esp
        
    _stop: # another infinite loop (after kernel infi loop) for security :)
        cli
        hlt
        jmp _stop

.section .data
    myString: 
        .ascii "\nYou are exited kernel!!! THIS IS NOT ACCCPEATABLE ! \n"

.section .stack
    .space 2*1024*1024 # move 2MB of space
kernel_stack: # just defining pointer to the end of 2MB file
