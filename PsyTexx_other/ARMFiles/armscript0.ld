MEMORY
{
    rom : ORIGIN = 0x00000000, LENGTH = 1M
}

ENTRY(ARM_PalmOS_main)

SECTIONS
{
    .text : { *(arm_startup) *(text) *(.gnu*) *(sec1) *(sec2) *(sec3) *(sec4)  }  > rom
    .alldata : { *(.bss) *(.bss.*) *(COMMON) *(.data) *(.rodata) *(.rodata.str1.4) }  > rom
    .rel : { *(.data.rel.local) *(.data.rel) } > rom
    .got : { *(.got) *(.got.plt) } > rom
}
