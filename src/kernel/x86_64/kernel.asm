%define MBSP_ADR 0x100000
%define IA32_EFER 0xC0000080
%define PML4T_BADR 0x1000000      ;0x20000;0x5000
%define KRLVIRADR 0x0
%define KINITSTACK_OFF 16
global _start
global x64_GDT
global kernel_pml4
extern kernel_main

[SECTION .text]
[BITS 32]
_start:
	cli
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
    lgdt [gdt_desc]
;enable PAE
    mov eax, cr4
    bts eax, 5                      ; CR4.PAE = 1
    mov cr4, eax
    mov eax, PML4T_BADR
    mov cr3, eax
;enable 64bits long-mode
    mov ecx, IA32_EFER
    rdmsr
    bts eax, 8                      ; IA32_EFER.LME =1
    wrmsr
;enable PE and paging
    mov eax, cr0
    bts eax, 0                      ; CR0.PE =1
    bts eax, 31
;enable CACHE
    btr eax,29		                ;CR0.NW=0
    btr eax,30		                ;CR0.CD=0  CACHE
    mov cr0, eax                    ; IA32_EFER.LMA = 1

    jmp 08:long_mode_entry


[BITS 64]
long_mode_entry:
	mov ax, 0x10						;data segment selector
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	xor rax, rax
	xor rbx, rbx
	xor rbp, rbp
	xor rcx, rcx
	xor rdx, rdx
	xor rdi, rdi
	xor rsi, rsi
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	xor r15, r15
	mov rsp, 0x9f000
	call kernel_main
    jmp $


[SECTION .data]
[BITS 32]
gdt_start_64:
null_x64_dsc: dq 0
krnl_c64_dsc: dq 0x0020980000000000           ; 64-bit kernel code seg
krnl_d64_dsc: dq 0x0000920000000000           ; 64-bit kernel data seg

user_c64_dsc: dq 0x0020f80000000000           ; 64-bit user code seg
user_d64_dsc: dq 0x0000f20000000000           ; 64-bit user data seg
gdt_end_64:

get_len	equ	gdt_end_64 - gdt_start_64			; length of gdt

gdt_desc:
    dw get_len - 1					; limit of gdt
	dq gdt_start_64

[section .data]

stack:
	times 1024 dq 0

kernel_pml4:
	times 512*10 dq 0
