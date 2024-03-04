#ifndef _APCI_H
#define _APCI_H

#include <type.h>

// Root System Description Pointer
typedef struct RSDP
{
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__((packed)) RSDP_t;

// eXtended System Descriptor Table
typedef struct XSDP
{
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress; // deprecated since version 2.0

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed)) XSDP_t;

typedef struct ACPISDTHeader
{
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__((packed)) ACPISDTHeader_t;

typedef struct RSDT
{
    ACPISDTHeader_t header;
    uint32_t PointerToOtherSDT[];
} __attribute__((packed)) RSDT_t;

typedef struct XSDT
{
    ACPISDTHeader_t header;
    uint64_t PointerToOtherSDT[];
} __attribute__((packed)) XSDT_t;

typedef struct MADT
{
    ACPISDTHeader_t header;
    uint32_t lapicaddr; // local apic address
    uint32_t flags;     // flags
    uint8_t entry[];
} __attribute__((packed)) MADT_t;

typedef struct APICHeader
{
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) APICHeader_t;

#define APIC_LOCAL 0         // local APIC
#define APIC_IO 1            // IO APIC
#define APIC_IO_ISO 2        // IO APIC interrupt source override
#define APIC_IO_NMIS 3       // IO APIC non-maskable interrupt source
#define APIC_LOCAL_NMI 4     // Local APIC non-maskable interrupt
#define APIC_LOCAL_ADR_OVR 5 // Local APIC Address Override
#define APIC_LOCAL_X2 9      // Processor Local x2APIC

// Processor Local APIC
typedef struct APIC0
{
    APICHeader_t header;
    uint8_t proc_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) localapic;

// I/O APIC
typedef struct APIC1
{
    APICHeader_t header;
    uint8_t apic_id;      // I/O APIC's ID
    uint8_t rsv;          // Reserved (0)
    uint32_t apic_addr;   // I/O APIC Address
    uint32_t sysint_base; // Global System Interrupt Base
} __attribute__((packed)) ioapic;

// I/O APIC Interrupt Source Override
typedef struct APIC2
{
    APICHeader_t header;
    uint8_t bus_src;  // Bus Source
    uint8_t irq_src;  // IRQ Source
    uint32_t sys_int; // Global System Interrupt
    uint16_t flags;   // Flags
} __attribute__((packed)) ioapic_iso;

typedef struct APIC3
{
    APICHeader_t header;
    uint8_t nmi_src;  // NMI Source
    uint8_t rsv;      // Reserved (0)
    uint16_t flags;   // Flags
    uint32_t sys_int; // Global System Interrupt
} __attribute__((packed)) ioapic_nmis;

typedef struct APIC4
{
    APICHeader_t header;
    uint8_t proc_id; // ACPI Processor ID (0xFF means all processors)
    uint16_t flags;  // Flags
    uint8_t lint;    // LINT# (0 or 1)
} __attribute__((packed)) lapic_nmi;

typedef struct APIC5
{
    APICHeader_t header;
    uint16_t rsv;       // Reserved (0)
    uint64_t apic_addr; //
} __attribute__((packed)) lapic_adr_ovr;

typedef struct APIC9
{
    APICHeader_t header;
    uint16_t rsv;
    uint32_t apic_id;
    uint32_t flags;
    uint32_t apci_id;
} __attribute__((packed)) lapic_x2;

void *search_rsdp();

MADT_t *search_madt();

MADT_t *parse_madt();

#endif