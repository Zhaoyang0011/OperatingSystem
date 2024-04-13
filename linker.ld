LINEAR_ADDRESS = 0x2000000;
ENTRY(_start)
OUTPUT_ARCH(i386:x86-64)
OUTPUT_FORMAT(elf64-x86-64)
SECTIONS
{
	. =LINEAR_ADDRESS;

    .text : ALIGN(4096)
    {
        *(.text)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
}