global _start
extern setup_main
extern setup_die

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

KERNEL_START equ 0x2000000
KERNEL_DESC_OFF equ -0x1000
E820_DESC equ 0x5000
E820_ADR equ E820_DESC + 4

[SECTION .text]
[BITS 16]

_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov sp, 0x7c00

    call get_mmap

.to_protected_mode: ; switch to protected mode
    cli
    lgdt[gdt_descriptor]

_before_protected:
    ; enable A20 line
    in al, 0x92
    or al, 0x2
    out 0x92, al

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG: protected_start

get_mmap:
    xor ebx, ebx
    xor esi, esi
    mov edi, E820_ADR

loop:
	mov eax, 0xe820
	mov ecx, 20
	mov edx, 0x534d4150
	int 0x15
    jc .1

    inc esi
    add edi, 20

	cmp ebx,0
	jne loop

	jmp .2

.1:
    xor esi, esi

.2:
	mov dword[E820_DESC], esi
	ret
    

[BITS 32]
protected_start:
    cli
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9fc00

    call chk_cpuid
;    call chk_cpu_long_mode

    mov ebx, 5
    mov ecx, 110
    mov edi, (KERNEL_START + KERNEL_DESC_OFF)
    call ata_lba_read

    call setup_main

    jmp CODE_SEG:KERNEL_START

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

chk_cpuid:
    pushfd                               ;Save EFLAGS
    pushfd                               ;Save EFLAGS
    xor dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                ;Load stored EFLAGS (with ID bit inverted)
    pushfd                               ;Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ;eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax,[esp]                        ;eax = whichever bits were changed
    popfd                                ;Restore original EFLAGS
    and eax,0x00200000                   ;eax = zero if ID bit can't be changed, else non-zero
    jz no_cpuid
    ret

; chk_cpu_long_mode:
;     mov eax, 0x80000000    ; Set the A-register to 0x80000000.
;     cpuid                  ; CPU identification.
;     cmp eax, 0x80000001    ; Compare the A-register with 0x80000001.
;     jb no_long_mode        ; It is less, there is no long mode.
;     mov eax, 0x80000001    ; Set the A-register to 0x80000001.
;     cpuid                  ; CPU identification.
;     test edx, 1 << 29      ; Test if the LM-bit, which is bit 29, is set in the D-register.
;     jz no_long_mode        ; They aren't, there is no long mode.
;     ret

no_cpuid:
no_long_mode:
    call setup_die

gdt_start:
gdt_null:
    dd 0x0
    dd 0x0
gdt_code:   ; cs point to this
    dw 0xffff   ; Limit 0-15
    dw 0x0      ; Base 0-15
    db 0x0      ; Base 16-23
    db 0x9a     ; Access byte
    db 0xcf     ; flags and limit 16-19
    db 0x0      ; Base 23-31
gdt_data:   ; ds, ss, fs, gs point to this
    dw 0xffff   ; Limit 0-15
    dw 0x0      ; Base 0-15
    db 0x0      ; Base 16-23
    db 0x92     ; Access byte
    db 0xcf     ; flags and limit 16-19
    db 0x0      ; Base 23-31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start