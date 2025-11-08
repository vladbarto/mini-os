#include "main.h"
#include "screen.h"
#include "logging.h"
#include "interrupt.h"

void KernelMain()
{   
    __enableSSE();  // only for demo; in the future will be called from __init.asm

    ClearScreen();

    InitLogging();

    Log("Logging initialized!");

    
    HelloBoot();
    idt_init();

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
    __magic();
    // TODO!!! PIC programming; see http://www.osdever.net/tutorials/view/programming-the-pic
    // TODO!!! define interrupt routines and dump trap frame
    

    // TODO!!! Timer programming

    // TODO!!! Keyboard programming

    // TODO!!! Implement a simple console

    // TODO!!! read disk sectors using PIO mode ATA

    // TODO!!! Memory management: virtual, physical and heap memory allocators
}
