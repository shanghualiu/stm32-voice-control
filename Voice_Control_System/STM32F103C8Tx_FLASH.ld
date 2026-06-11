/*****************************************************************************
 * Linker script for STM32F103C8T6 (Blue Pill)
 * - Flash: 64KB (origin 0x08000000)
 * - RAM:   20KB (origin 0x20000000)
 *
 * Adapted from STM32CubeF1 GCC linker script
 *****************************************************************************/

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Stack and Heap sizes */
_estack     = ORIGIN(RAM) + LENGTH(RAM);    /* End of RAM (top of stack) */
_Min_Stack_Size  = 0x400;                   /* 1KB minimum stack */
_Min_Heap_Size   = 0x200;                   /* 512B minimum heap */

SECTIONS
{
    /* ---- Vector table and code ---- */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;          /* End of text section */
    } > FLASH

    /* ---- Constant data ---- */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* ---- ARM exception unwinding (not used) ---- */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* ---- Pre-init array ---- */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH
    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH
    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH

    /* ---- Initialized data (load from Flash) ---- */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;          /* Start of data in RAM */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;          /* End of data in RAM */
    } > RAM AT > FLASH

    /* ---- Uninitialized data (BSS) ---- */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;           /* Start of BSS */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;           /* End of BSS */
        __bss_end__ = _ebss;
    } > RAM

    /* ---- Heap and Stack ---- */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM

    /* ---- Debug info (discarded from final binary) ---- */
    /DISCARD/ :
    {
        libc.a(*)
        libm.a(*)
        libgcc.a(*)
    }
}
