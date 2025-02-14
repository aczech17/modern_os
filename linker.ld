ENTRY(stage1)

PHDRS
{
    text PT_LOAD FLAGS(5);  /* R-X */
    data PT_LOAD FLAGS(6);  /* RW- */
}

SECTIONS
{
    . = 0x00000000;
    .stage1 : {
        KEEP(*(.stage1))
    }

    . = 0x00000200;
    .stage2 : {
        KEEP(*(.stage2))
    }

    . = 0x00100000;
    .text : ALIGN(4K) {
        *(.text)
    } :text

    .rodata : ALIGN(4K) {
        *(.rodata)
    } :text

    .data : ALIGN(4K) {
        *(.data)
    } :data

    .bss : ALIGN(4K) {
        *(.bss)
    } :data
}
