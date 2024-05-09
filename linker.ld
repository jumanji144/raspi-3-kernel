ENTRY(_start)

MEMORY {
    FLASH (rx) : ORIGIN = 0x80000, LENGTH = 100k
    RAM (rwx) : ORIGIN = 0x300000, LENGTH = 30M
}

SECTIONS
{
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)

        . = ALIGN(8);
    } >FLASH
 
    .rodata :
    {
        . = ALIGN(8);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(8);
    } >FLASH

    __data_load__ = LOADADDR(.data);

    /* Initialized data sections into "RAM" Ram type memory */
    .data :
    {
        . = ALIGN(8);
        __data_start__ = .;        /* create a global symbol at data start */
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        *(.RamFunc)        /* .RamFunc sections */
        *(.RamFunc*)       /* .RamFunc* sections */

        . = ALIGN(8);
        __data_end__ = .;        /* define a global symbol at data end */

    } >RAM AT> FLASH

    /* Uninitialized data section into "RAM" Ram type memory */
    . = ALIGN(8);
    .bss :
    {
    /* This is used by the startup in order to initialize the .bss section */
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(8);
    __bss_end__ = .;
    } >RAM
}