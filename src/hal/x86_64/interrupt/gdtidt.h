#ifndef _GDT_IDT_H
#define _GDT_IDT_H

#include <hal/halglobal.h>
#include <type.h>

#define IDT_MAX 256

/* GDT */
/* 描述符索引 */
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

#define INDEX_DUMMY 0
#define INDEX_FLAT_C 1
#define INDEX_FLAT_RW 2
/* 选择子 */
#define SELECTOR_DUMMY 0
#define SELECTOR_FLAT_C 0x08
#define SELECTOR_FLAT_RW 0x10

#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW

/* 描述符类型值说明 */
#define DA_64 0x2000
#define DA_32 0x4000       // 32 位段
#define DA_LIMIT_4K 0x8000 // 段界限粒度为 4K 字节
#define DA_DPL0 0x00       // DPL = 0
#define DA_DPL1 0x20       // DPL = 1
#define DA_DPL2 0x40       // DPL = 2
#define DA_DPL3 0x60       // DPL = 3
/* 存储段描述符类型值说明 */
#define DA_DR 0x90   // 存在的只读数据段类型值
#define DA_DRW 0x92  // 存在的可读写数据段属性值
#define DA_DRWA 0x93 // 存在的已访问可读写数据段类型值
#define DA_C 0x98    // 存在的只执行代码段属性值
#define DA_CR 0x9A   // 存在的可执行可读代码段属性值
#define DA_CCO 0x9C  // 存在的只执行一致代码段属性值
#define DA_CCOR 0x9E // 存在的可执行可读一致代码段属性值
/* 系统段描述符类型值说明 */
#define DA_LDT 0x82      // 局部描述符表段类型值
#define DA_TaskGate 0x85 // 任务门类型值
#define DA_386TSS 0x89   // 可用 386 任务状态段类型值
#define DA_386CGate 0x8C // 386 调用门类型值
#define DA_386IGate 0x8E // 386 中断门类型值
#define DA_386TGate 0x8F // 386 陷阱门类型值

/* 中断向量 */
#define INT_VECTOR_DIVIDE 0x0   // divide 0
#define INT_VECTOR_DEBUG 0x1
#define INT_VECTOR_NMI 0x2
#define INT_VECTOR_BREAKPOINT 0x3
#define INT_VECTOR_OVERFLOW 0x4
#define INT_VECTOR_BOUNDS 0x5
#define INT_VECTOR_INVAL_OP 0x6
#define INT_VECTOR_COPROC_NOT 0x7
#define INT_VECTOR_DOUBLE_FAULT 0x8
#define INT_VECTOR_COPROC_SEG 0x9
#define INT_VECTOR_INVAL_TSS 0xA
#define INT_VECTOR_SEG_NOT 0xB
#define INT_VECTOR_STACK_FAULT 0xC
#define INT_VECTOR_PROTECTION 0xD
#define INT_VECTOR_PAGE_FAULT 0xE
#define INT_VECTOR_COPROC_ERR 0x10
#define INT_VECTOR_ALIGN_CHEK 0x11
#define INT_VECTOR_MACHI_CHEK 0x12
#define INT_VECTOR_SIMD_FAULT 0x13

/* 中断向量 */
#define INT_VECTOR_IRQ0 0x20
#define INT_VECTOR_IRQ8 0x28

#define INT_VECTOR_APIC_IPI_SCHEDUL 0xe0
#define INT_VECTOR_APIC_SVR 0xef
#define INT_VECTOR_APIC_TIMER 0xf0
#define INT_VECTOR_APIC_THERMAL 0xf1
#define INT_VECTOR_APIC_PERFORM 0xf2
#define INT_VECTOR_APIC_LINTO 0xf3
#define INT_VECTOR_APIC_LINTI 0xf4
#define INT_VECTOR_APIC_ERROR 0xf5

#define INT_VECTOR_SYSCALL 0xFF

// Interrupt descriptor
typedef struct gate
{
    uint16_t offset1;  // Offset 0-15
    uint16_t selector; // Code Selector
    uint8_t dcount;    /* 该字段只在调用门描述符中有效。如果在利用
               调用门调用子程序时引起特权级的转换和堆栈
               的改变，需要将外层堆栈中的参数复制到内层
               堆栈。该双字计数字段就是用于说明这种情况
               发生时，要复制的双字参数的数量。*/
    uint8_t attr;      // P(1) DPL(2) DT(1) TYPE(4)
    uint16_t offset2;  // Offset 16 - 31
    uint32_t offset3;  // Offset 32 - 63
    uint32_t resv;     // reserved bits
} __attribute__((packed)) gate_t;

typedef struct idt_reg
{
    uint16_t idtLen;
    uint64_t idtbass;
} __attribute__((packed)) idtr_t;

typedef void (*int_handler_t)();

void init_idt();

HAL_DEFGLOB_VARIABLE(gate_t, x64_idt)[IDT_MAX];
HAL_DEFGLOB_VARIABLE(idtr_t, idtr);

#endif