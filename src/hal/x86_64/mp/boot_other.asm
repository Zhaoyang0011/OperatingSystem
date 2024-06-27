ORG 0x7c00

SETUP_START equ 0x7000

[section .text]
[BITS 16]
start:
    cli
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov sp, 0x7c00

    mov ebx, 1
    mov ecx, 11
    mov edi, SETUP_START
    call ata_lba_read

    jmp 0x0:SETUP_START

    jmp $

times 510 - ($ - $$) db 0
db 0x55, 0xaa