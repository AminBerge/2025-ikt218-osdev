ENTRY(_start)

SECTIONS {
    /* Start kernel at 1MB (0x00100000) */
    . = 1M;

    /* Multiboot2 header must be in the first 8192 bytes of the kernel
       and aligned to 8 bytes. */
    .boot ALIGN(8) :
    {
        *(.multiboot_header)
    }

    . = ALIGN(4K);
    .text :
    {
        *(.text .text.*)
    }0

    . = ALIGN(4K);
    .rodata :
    {
        *(.rodata .rodata.*)
    }

    . = ALIGN(4K);
    .data :
    {
        *(.data .data.*)
    }

    . = ALIGN(4K);
    .bss :
    {
        *(.bss .bss.*)
    }

    end = .;
    _end = .;
    __end = .;
}
