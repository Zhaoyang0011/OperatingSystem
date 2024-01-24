LINEAR_ADDRESS = 0x2000000;
ENTRY(_start)
OUTPUT_ARCH(i386:x86-64)
OUTPUT_FORMAT(elf64-x86-64)
SECTIONS
{
	. =LINEAR_ADDRESS;

    .text : ALIGN(16)
    {
        *(.text)
    }

    .rodata : ALIGN(16)
    {
        *(.rodata)
    }

    .data : ALIGN(16)
    {
        *(.data)
    }

    .bss : ALIGN(16)
    {
        *(COMMON)
        *(.bss)
    }
}