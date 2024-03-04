extern hal_hwint_allocator
extern hal_fault_allocator
extern hal_syscl_allocator

global	exc_divide_error
global	exc_single_step_exception
global	exc_nmi
global	exc_breakpoint_exception
global	exc_overflow
global	exc_bounds_check
global	exc_inval_opcode
global	exc_copr_not_available
global	exc_double_fault
global	exc_copr_seg_overrun
global	exc_inval_tss
global	exc_segment_not_present
global	exc_stack_exception
global	exc_general_protection
global	exc_page_fault
global	exc_copr_error
global	exc_alignment_check
global	exc_machine_check
global	exc_simd_fault
global	hxi_exc_general_intpfault
global	hxi_hwint00
global	hxi_hwint01
global	hxi_hwint02
global	hxi_hwint03
global	hxi_hwint04
global	hxi_hwint05
global	hxi_hwint06
global	hxi_hwint07
global	hxi_hwint08
global	hxi_hwint09
global	hxi_hwint10
global	hxi_hwint11
global	hxi_hwint12
global	hxi_hwint13
global	hxi_hwint14
global	hxi_hwint15
global	hxi_hwint16
global	hxi_hwint17
global	hxi_hwint18
global	hxi_hwint19
global	hxi_hwint20
global	hxi_hwint21
global	hxi_hwint22
global	hxi_hwint23
global  hxi_apic_ipi_schedul
global  hxi_apic_svr
global  hxi_apic_timer
global  hxi_apic_thermal
global  hxi_apic_performonitor
global  hxi_apic_lint0
global  hxi_apic_lint1
global  hxi_apic_error
global	exi_sys_call
global  asm_ret_from_user_mode

%define _NOERRO_CODE 0xffffffffffffffff

%macro	SAVEALL	0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	
	xor r14,r14
	mov r14w,ds
	push r14
	mov r14w,es
	push r14
	mov r14w,fs
	push r14
	mov r14w,gs
	push r14
%endmacro


%macro	RESTOREALL	0
	pop r14
	mov gs,r14w
	pop r14 
	mov fs,r14w
	pop r14
	mov es,r14w
	pop r14
	mov ds,r14w
	 
	
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	
	iretq
%endmacro

%macro	SAVEALLFAULT 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	xor r14,r14
	mov r14w,ds
	push r14
	mov r14w,es
	push r14
	mov r14w,fs
	push r14
	mov r14w,gs
	push r14
%endmacro

%macro	RESTOREALLFAULT	0
	pop r14
	mov gs,r14w
	pop r14 
	mov fs,r14w
	pop r14
	mov es,r14w
	pop r14
	mov ds,r14w
	
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	
	add rsp,8

	iretq
%endmacro

%macro	SRFTFAULT 1
	push	  _NOERRO_CODE
	SAVEALLFAULT
	mov r14w,0x10
	mov ds,r14w
	mov es,r14w
	mov fs,r14w
	mov gs,r14w

	mov rdi,%1 ;rdi, rsi
	mov rsi,rsp
	call hal_fault_allocator
	RESTOREALLFAULT
%endmacro

%macro	SRFTFAULT_ECODE 1
	SAVEALLFAULT
	mov r14w,0x10
	mov ds,r14w
	mov es,r14w
	mov fs,r14w
	mov gs,r14w

	mov rdi,%1
	mov rsi,rsp
	call hal_fault_allocator
	RESTOREALLFAULT
%endmacro

%macro	HARWINT	1
	SAVEALL
	
	mov r14w,0x10
	mov ds,r14w
	mov es,r14w
	mov fs,r14w
	mov gs,r14w

	mov	rdi, %1
	mov rsi,rsp
	call hal_hwint_allocator

	RESTOREALL
%endmacro

%macro  EXI_SCALL  0
	
	;push rax //rax is used for parameter
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	
	xor r14,r14
	
	mov r14w,ds
	push r14
	mov r14w,es
	push r14
	mov r14w,fs
	push r14
	mov r14w,gs
	push r14
	
	mov r14w,0x10
	mov ds,r14w
	mov es,r14w
	mov fs,r14w
	mov gs,r14w

	mov	rdi, rax
	mov rsi, rsp
	call hal_syscl_allocator
	
	pop r14
	mov gs,r14w
	pop r14 
	mov fs,r14w
	pop r14
	mov es,r14w
	pop r14
	mov ds,r14w
	 
	
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	;pop rax
	
	iretq
%endmacro

[section .hwint.text]
[BITS 64]

ALIGN	16
exc_divide_error:
	SRFTFAULT 0
ALIGN	16
exc_single_step_exception:
	SRFTFAULT 1
ALIGN	16
exc_nmi:
	SRFTFAULT 2
ALIGN	16
exc_breakpoint_exception:
	SRFTFAULT 3
ALIGN	16
exc_overflow:
	SRFTFAULT 4
ALIGN	16
exc_bounds_check:
	SRFTFAULT 5
ALIGN	16
exc_inval_opcode:
	SRFTFAULT 6
ALIGN	16
exc_copr_not_available:
	SRFTFAULT 7
ALIGN	16
exc_double_fault:
	SRFTFAULT_ECODE 8
ALIGN	16
exc_copr_seg_overrun:
	SRFTFAULT 9
ALIGN	16
exc_inval_tss:
	SRFTFAULT_ECODE 10
ALIGN	16
exc_segment_not_present:
	SRFTFAULT_ECODE 11
ALIGN	16
exc_stack_exception:
	SRFTFAULT_ECODE 12
ALIGN	16
exc_general_protection:
	SRFTFAULT_ECODE 13
ALIGN	16
exc_page_fault:
	SRFTFAULT_ECODE 14
ALIGN	16
exc_copr_error:
	SRFTFAULT 16
ALIGN	16
exc_alignment_check:
	SRFTFAULT_ECODE 17
ALIGN	16
exc_machine_check:
	SRFTFAULT 18
ALIGN	16
exc_simd_fault:
	SRFTFAULT 19
ALIGN	16
hxi_exc_general_intpfault:
	SRFTFAULT 256

