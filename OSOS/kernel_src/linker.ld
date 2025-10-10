ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

/* define the program headers (segments) to just DONT give W and E permission to same section*/
PHDRS
{
    text_segment PT_LOAD FLAGS(5); /* R_E = 5 */
    data_segment PT_LOAD FLAGS(6); /* RW_ = 6 */
}
/* Note :  one segment can contain many sections. */

SECTIONS
{
    . = 0x0100000;
    
    /* assign sections to the 'text_segment' program header */
    .text :
    {
        *(.multiboot)
        *(.text*)
        *(.rodata)

    } :text_segment

    .ctors : ALIGN(4)
    {
        ctors_start = .;
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        ctors_end = .;

    } :text_segment

    /*.dtors are not currently in use, may use in future*/
    .dtors : ALIGN(4)
    {
        dtors_start = .;
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        dtors_end = .;

    } :text_segment 

    /* assign sections to the 'data_segment' program header */

    .data :
    {
        *(.data)

    } :data_segment

    .bss :
    {
        *(.bss)

    } :data_segment
    
    .stack (NOLOAD) : { 
        *(.stack)

    } :data_segment 

    /DISCARD/ : 
    {
        *(.comment)
        *(.eh_frame)
        *(.got.plt)
    }
}