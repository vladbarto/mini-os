#include "interrupt.h"

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

static bool vectors[IDT_MAX_DESCRIPTORS];

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    __magic();
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}

void InterruptCommonHandler(
    INTERRUPT_STACK_COMPLETE* StackPointer, // Pointer to Stack Pointer After Transfer to Handler (Fig 6-9)
    uint8_t                   ErrorCodeAvailable, // 0 if not available
    uint8_t                   InterruptIndex, // [0x0, 0xFF]
    COMPLETE_PROCESSOR_STATE* ProcessorState // Pointer to a structure which contains trap context (see above trap frame dump example
) {
   
}