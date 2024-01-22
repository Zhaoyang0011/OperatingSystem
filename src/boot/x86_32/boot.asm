ORG 0x7c00              ;bootloader代码会被加载到0x7c00

SETUP_START equ 0x500

[section .text]
[BITS 16]
_start:
    jmp short start
    nop
    times 0x21 db 0

start:
    cli
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov sp, 0x7c00
    sti

    mov ebx, 1      ;
    mov ecx, 4      ;
    mov edi, SETUP_START
    call ata_lba_read

    jmp 0x0:SETUP_START

ata_lba_read:
    mov eax, ebx
    ; LBA low
    mov dx, 0x1f3
    out dx, al

    ; LBA mid
    mov dx, 0x1f4
    shr eax, 8
    out dx, al

    ; LBA high
    mov dx, 0x1f5
    shr eax, 16
    out dx, al

    mov dx, 0x1f6
    shr eax, 8
    and al, 0x0f
    or al, 0xe0
    out dx, al
    
    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.next_sector:
    push ecx

    mov dx, 0x1f7
.check_hd:
    in al, dx
    test al, 8
    jz .check_hd

.read_hd:
    mov ecx, 256
    mov dx, 0x1f0
    rep insw

    pop ecx
    loop .next_sector

    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa