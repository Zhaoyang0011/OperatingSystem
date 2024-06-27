#ifndef _APIC_H
#define _APIC_H

#include <hal/halglobal.h>
#include <type.h>

#include "../apci.h"

// FEE0 0000H Reserved
// FEE0 0010H Reserved
// FEE0 0020H Local APIC ID Register Read/Write.
// FEE0 0030H Local APIC Version Register Read Only.
// FEE0 0040H Reserved
// FEE0 0050H Reserved
// FEE0 0060H Reserved
// FEE0 0070H Reserved
// FEE0 0080H Task Priority Register (TPR) Read/Write.
// FEE0 0090H Arbitration Priority Register1 (APR) Read Only.
// FEE0 00A0H Processor Priority Register (PPR) Read Only.
// FEE0 00B0H EOI Register Write Only.
// FEE0 00C0H Remote Read Register1 (RRD) Read Only
// FEE0 00D0H Logical Destination Register Read/Write.
// FEE0 00E0H Destination Format Register Read/Write (see Section
// 10.6.2.2).
// FEE0 00F0H Spurious Interrupt Vector Register Read/Write (see Section 10.9.
// FEE0 0100H In-Service Register (ISR); bits 31:0 Read Only.
// FEE0 0110H In-Service Register (ISR); bits 63:32 Read Only.
// FEE0 0120H In-Service Register (ISR); bits 95:64 Read Only.
// FEE0 0130H In-Service Register (ISR); bits 127:96 Read Only.
// FEE0 0140H In-Service Register (ISR); bits 159:128 Read Only.
// FEE0 0150H In-Service Register (ISR); bits 191:160 Read Only.
// FEE0 0160H In-Service Register (ISR); bits 223:192 Read Only.
// FEE0 0170H In-Service Register (ISR); bits 255:224 Read Only.
// FEE0 0180H Trigger Mode Register (TMR); bits 31:0 Read Only.
// FEE0 0190H Trigger Mode Register (TMR); bits 63:32 Read Only.
// FEE0 01A0H Trigger Mode Register (TMR); bits 95:64 Read Only.
// FEE0 01B0H Trigger Mode Register (TMR); bits 127:96 Read Only.
// FEE0 01C0H Trigger Mode Register (TMR); bits 159:128 Read Only.
// FEE0 01D0H Trigger Mode Register (TMR); bits 191:160 Read Only.

// Local APIC registers, divided by 4 for use as uint[] indices.
#define LAPIC_ID (0x0020 / 4)       // ID
#define LAPIC_VER (0x0030 / 4)      // Version
#define LAPIC_TPR (0x0080 / 4)      // Task Priority
#define LAPIC_EOI (0x00B0 / 4)      // EOI
#define LAPIC_SVR (0x00F0 / 4)      // Spurious Interrupt Vector
#define LAPIC_SVR_ENABLE 0x00000100 // Unit Enable
#define LAPIC_ESR (0x0280 / 4)      // Error Status

#define LAPIC_ICRLO (0x0300 / 4)       // Interrupt Command
#define LAPIC_ICRLO_INIT 0x00000500    // INIT/RESET
#define LAPIC_ICRLO_STARTUP 0x00000600 // Startup IPI
#define LAPIC_ICRLO_DELIVS 0x00001000  // Delivery status
#define LAPIC_ICRLO_ASSERT 0x00004000  // Assert interrupt (vs deassert)
#define LAPIC_ICRLO_DEASSERT 0x00000000
#define LAPIC_ICRLO_LEVEL 0x00008000 // Level triggered
#define LAPIC_ICRLO_BCAST 0x00080000 // Send to all APICs, including self.
#define LAPIC_ICRLO_BUSY 0x00001000
#define LAPIC_ICRLO_FIXED 0x00000000

#define LAPIC_ICRHI (0x0310 / 4) // Interrupt Command [63:32]

#define LAPIC_TIMER (0x0320 / 4)        // Local Vector Table 0 (TIMER)
#define LAPIC_TIMER_X1 0x0000000B       // divide counts by 1
#define LAPIC_TIMER_PERIODIC 0x00020000 // Periodic

#define LAPIC_PCINT (0x0340 / 4) // Performance Counter LVT
#define LAPIC_LINT0 (0x0350 / 4) // Local Vector Table 1 (LINT0)
#define LAPIC_LINT1 (0x0360 / 4) // Local Vector Table 2 (LINT1)
#define LAPIC_ERROR (0x0370 / 4) // Local Vector Table 3 (ERROR)

#define LAPIC_MASKED 0x00010000 // Interrupt masked

#define LAPIC_TIMER_ICR (0x0380 / 4) // Timer Initial Count
#define LAPIC_TIMER_CCR (0x0390 / 4) // Timer Current Count
#define LAPIC_TIMER_DCR (0x03E0 / 4) // Timer Divide Configuration

#define T_IRQ0 0x20 // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER 0
#define IRQ_KBD 1
#define IRQ_COM1 4
#define IRQ_IDE 14
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31

HAL_DEFGLOB_VARIABLE(uint32_t *, lapic);

void find_apic(MADT_t *madt);

void init_lapic();

// write lapic register
KLINE void lapicw(uint32_t index, uint32_t value) {
  lapic[index] = value;
  lapic[LAPIC_ID];
}

#endif