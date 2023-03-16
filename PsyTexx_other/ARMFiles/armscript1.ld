MEMORY
{
    ram : ORIGIN = 0x00000000, LENGTH = 1M
}

ENTRY(ARM_PalmOS_main)

SECTIONS
{
    .text : { *(arm_startup) *(text) *(.gnu*) *(sec1) *(sec2) *(sec3) *(sec4)  }  > ram
    .alldata : { *(.bss) *(.bss.*) *(COMMON) *(.data) *(.rodata) *(.rodata.*) }  > ram
    .rel : { *(.data.rel.local) *(.data.rel) *(.data.rel.*) } > ram
    .got : { *(.got) *(.got.plt) } > ram
}
