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

    .ctors_dtors : ALIGN(4)
    {
        start_ctors = .;
        KEEP(*(.init_array));
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        end_ctors = .;
        
        start_dtors = .;
        KEEP(*(.fini_array));
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)));
        end_dtors = .;

    } :text_segment /* constructors are also part of the read-only segment */

    /* assign sections to the 'data_segment' program header */
    .data :
    {
        *(.data)
    } :data_segment

    .bss :
    {
        *(.bss)
    } :data_segment /* BSS is also part of the read-write segment */
    
    .stack (NOLOAD) : { 
        *(.stack)
    } :data_segment /* stack is also part of the read-write segment */

    /DISCARD/ : 
    {
        *(.comment)
        *(.eh_frame)
        *(.got.plt)
    }
}