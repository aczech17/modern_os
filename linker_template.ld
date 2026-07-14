ENTRY(kernel_main)

PHDRS
{
    text PT_LOAD FLAGS(5);  /* R-X */
    data PT_LOAD FLAGS(6);  /* RW- */
}

SECTIONS
{
    . = TEXT_ADDR;
    .text : ALIGN(4K)
    {
        *(.text)
    } :text

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    } :text

    .data : ALIGN(4K)
    {
        *(.data)
    } :data
}
