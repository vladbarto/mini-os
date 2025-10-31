#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "main.h"

#define IDT_MAX_DESCRIPTORS 32
#define GDT_OFFSET_KERNEL_CODE 8
#define GDT_OFFSET_KERNEL_DATA 16

typedef struct _IDT_ENTRY {
	uint16_t    isr_low;      // Bits 15:0 of address -- The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

typedef struct _INTERRUPT_STACK_COMPLETE{
    uint64_t SS;
    uint64_t RSP;
    uint64_t RFLAGS;
    uint64_t CS;  // optional, only if privilege change
    uint64_t RIP;   // optional, only if privilege change
    uint64_t ErrorCode; // present only for exceptions with error code
} INTERRUPT_STACK_COMPLETE;

typedef struct _COMPLETE_PROCESSOR_STATE {
    int64_t RAX;
    int64_t RCX;
    int64_t RDX;
    int64_t RBX;
    int64_t RSI;
    int64_t RDI;
    int64_t RBP;
    int64_t R8;
    int64_t R9;
    int64_t R10;
    int64_t R11;
    int64_t R12;
    int64_t R13;
    int64_t R14;
    int64_t R15;
    // int64_t DS;
    // int64_t ES;
    int64_t FS;
    int64_t GS;
} COMPLETE_PROCESSOR_STATE;

extern void* isr_stub_table[];

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance
static idtr_t idtr;
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void idt_init(void);

void InterruptCommonHandler(
    INTERRUPT_STACK_COMPLETE* StackPointer, // Pointer to Stack Pointer After Transfer to Handler (Fig 6-9)
    uint8_t                   ErrorCodeAvailable, // 0 if not available
    uint8_t                   InterruptIndex, // [0x0, 0xFF]
    COMPLETE_PROCESSOR_STATE* ProcessorState // Pointer to a structure which contains trap context (see above trap frame dump example
);
#endif // _INTERRUPT_H_