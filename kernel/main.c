#include "main.h"

void KernelMain()
{   
    __enableSSE();  // only for demo; in the future will be called from __init.asm

    ClearScreen();

    InitLogging();

    Log("Logging initialized!\n");

    HelloBoot();

    PIC_remap(0x20, 0x28);
    Log("PIC remapped (master=0x20, slave=0x28)\n");

    PIC_disable();
    Log("All IRQs masked with 0xFF.\n");

    idt_init();
    Log("IDT initialized.\n");
    // Program Timer (aka PIT)
    PIT_init(100);
    PS2_keyboard_init();
    
    CLI_init();

    ata_init();
    
    PIC_unmask(0); // IRQ0 = timer
    PIC_unmask(1); // IRQ1 = keyboard

/* Uncomment each one alternatively to test IDT functionality. Both uncommented will not work */
    // division by 0; generate #DE
    // int DivisionByZero = 23/0;
    // __magic();

    // generate #NP
//     __asm__ __volatile__(
//     ".intel_syntax noprefix\n\t"
//     "mov ax, 0x18\n\t"
//     "mov ds, ax\n\t"
//     ".att_syntax prefix\n\t"
// );
    // TODO!!! read disk sectors using PIO mode ATA

    // TODO!!! Memory management: virtual, physical and heap memory allocators
}
