OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
  . = 0x0;

  .text : {
    *(.text.start)
    *(.text .text.*)
  }

  .rodata : {
    *(.rodata .rodata.*)
  }

  .data : {
    *(.data .data.*)
  }

  .bss : {
    *(.bss .bss.*)
    *(COMMON)
  }
}
