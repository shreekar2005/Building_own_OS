ENTRY(loader) 
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
    . = 0x0100000;
    
    .text :
    {
        *(.multiboot)
        *(.text*)
        *(.rodata)
    }

    /* A dedicated, read-only section for constructors */
    .ctors : ALIGN(4)
    {
        start_ctors = .;
        KEEP(*(.init_array));
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        end_ctors = .;
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
    
    /* Now this stack section is not actually loaded to executable file (to prevent larger size). This is just name to use in future if required */
    .stack (NOLOAD) : { 
        *(.stack)
    }

    /DISCARD/ : 
    {
        *(.comment)
        *(.eh_frame)
        *(.got.plt)
    }
}