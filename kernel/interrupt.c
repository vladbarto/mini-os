#include "interrupt.h"

__attribute__((aligned(16)))
idt_entry_t idt[256];

idtr_t idtr;

volatile uint64_t g_TickCount = 0;
// extern const char* _kkybrd_scancode_std; // normal scancode-to-char table
// extern const char* _kkybrd_scancode_ext; // extended scancode-to-char table
static uint8_t kbd_ext_state = 0; // 0 = none, 0xE0 or 0xE1 if prefix seen

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

// --------------------------------- PRIVATE FUNCTION HEADER DECLARATIONS ----------------------
void __ExceptionHandler(
    INTERRUPT_STACK_COMPLETE* StackPointer,
    uint8_t                   ErrorCodeAvailable,
    uint8_t                   InterruptIndex,
    COMPLETE_PROCESSOR_STATE* ProcessorState 
);
void __IRQ0_TimerHandler();
void __IRQ1_KeyboardHandler();
void __InterpretCommand(char* cmd);
// ---------------------------------------------------------------------------------------------

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < IDT_MAX_DESCRIPTORS; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    //__asm__ volatile ("sti"); // set the interrupt flag
}

void InterruptCommonHandler(
    INTERRUPT_STACK_COMPLETE* StackPointer, // Pointer to Stack Pointer After Transfer to Handler (Fig 6-9)
    uint8_t                   ErrorCodeAvailable, // 0 if not available
    uint8_t                   InterruptIndex, // [0x0, 0xFF]
    COMPLETE_PROCESSOR_STATE* ProcessorState // Pointer to a structure which contains trap context (see above trap frame dump example
) {
    if (InterruptIndex < 32) {
        // this is an exception
        __ExceptionHandler(StackPointer, ErrorCodeAvailable, InterruptIndex, ProcessorState);
    } else  {
        uint8_t irq = InterruptIndex - 32;

        switch (irq) {
        case 0: // Timer
            //__IRQ0_TimerHandler();
            break;

        case 1: // Keyboard IRQ
            __IRQ1_KeyboardHandler();
            break;

        default:
            break;
        }

        PIC_sendEOI(irq);
    }
}

// ------------------------ PRIVATE METHODS CODE ------------------------------
void __ExceptionHandler(
    INTERRUPT_STACK_COMPLETE* StackPointer,
    uint8_t                   ErrorCodeAvailable,
    uint8_t                   InterruptIndex,
    COMPLETE_PROCESSOR_STATE* ProcessorState 
) {

    // quick sanity prints to verify pointers/values
    LogSerialAndScreen("Trap Info:\n");
    LogSerialAndScreen("---------------\n");
    LogSerialAndScreen("stack pointer = %X\n", StackPointer); //GOOD
    LogSerialAndScreen("processor state pointer = %X\n", ProcessorState); //GOOD
    LogSerialAndScreen("ErrorCodeAvailable = %D\n", ErrorCodeAvailable);
    LogSerialAndScreen("InterruptIndex = %D\n", InterruptIndex);
    LogSerialAndScreen("---------------\n");
    LogSerialAndScreen("Registers:\n");    
    LogSerialAndScreen("rax=%X rbx=%X rcx=%X\n",
        ProcessorState->RAX,
        ProcessorState->RBX,
        ProcessorState->RCX);
    LogSerialAndScreen("rdx=%X rsi=%X rdi=%X\n",
            ProcessorState->RDX,
            ProcessorState->RSI,
            ProcessorState->RDI);
    LogSerialAndScreen("rip=%X rsp=%X rbp=%X\n",
        StackPointer->RIP,
        StackPointer->RSP,
        ProcessorState->RBP);
    LogSerialAndScreen("r8=%X r9=%X r10=%X\n",
        ProcessorState->R8,
        ProcessorState->R9,
        ProcessorState->R10);  
    LogSerialAndScreen("r11=%X r12=%X r13=%X\n",
        ProcessorState->R11,
        ProcessorState->R12,
        ProcessorState->R13); 
    LogSerialAndScreen("r14=%X r15=%X\n",
        ProcessorState->R14,
        ProcessorState->R15); 
    LogSerialAndScreen("cs=%X ss=%X fs=%X gs=%X rflags=%X\n\n",
        StackPointer->CS,
        StackPointer->SS,
        ProcessorState->FS,
        ProcessorState->GS,
        StackPointer->RFLAGS); 

    // // Stack dump: read memory at the interrupted stack pointer (StackPointer->RSP)
    LogSerialAndScreen("Stack (first 10 qwords) from interrupted RSP:\n");
    LogSerialAndScreen("|-Address-|-Value-|--|-Address-|-Value-|\n");
    {
        uint64_t *sp = (uint64_t*)(uintptr_t)StackPointer->RSP;
        for (int i = 0; i < 10; i += 2) {
            uint64_t val = 0, val2;
            // read memory at sp + i
            val = *(sp - i*8);
            val2 = *(sp - i*16);
            LogSerialAndScreen("%X: %X    |     %X: %X\n", StackPointer->RSP - i*8, val, StackPointer->RSP - i*16, val2);
        }
    }

    LogSerialAndScreen("---- trap done ----\n");
    
    __asm__ volatile ("CLI"); // clear the interrupt flag
    __asm__ volatile ("HLT"); // halt the processor
    ClearScreen();
}

void __IRQ0_TimerHandler() {

    g_TickCount++;
    if (g_TickCount % 100 == 0) {
        LogSerialAndScreen("Timer tick: %u\n", (uint32_t)g_TickCount);
    }
}

void __IRQ1_KeyboardHandler() {

    // only read if output buffer full
    uint8_t status = __inbyte(PS2_STATUS_PORT);
    if (status & PS2_STATUS_OUTPUT_BUFFER_FULL) {
        uint8_t sc = __inbyte(PS2_DATA_PORT);

        // handle extended prefixes
        if (sc == 0xE0 || sc == 0xE1) {
            kbd_ext_state = sc; // wait for next byte(s)
        } else {
            bool released = (sc & 0x80) != 0; // isKeyReleased
            uint8_t sc_masked = sc & 0x7F;
            char ch = 0;
            if (kbd_ext_state) {
                ch = _kkybrd_scancode_ext[sc_masked];
                kbd_ext_state = 0;
            } else {
                ch = _kkybrd_scancode_std[sc_masked];
            }

            if (!released && ch) {
                if (ch == '\n' || ch == '\r' || ch == KEY_KP_ENTER)
                {
                    FormattedLog("recorded command = %s\n", BufferCLI);
                    __InterpretCommand(BufferCLI);
                    cl_flush(BufferCLI, MAX_COLUMNS);
                }
                // display printable char
                else {
                    //cl_snprintf(BufferCLI, MAX_COLUMNS, "%s%c", BufferCLI, ch);
                    size_t len = cl_strlen(BufferCLI);
                    if (len < MAX_COLUMNS - 1) {
                        BufferCLI[len] = ch;
                        BufferCLI[len + 1] = '\0';
                    }
                    LogSerialAndScreen("%c", ch);
                }
            }
       }
    }
}

void __InterpretCommand(char* cmd) {
    FormattedLog("[CMD_INTERPRETOR] COMMAND = %s\n", cmd);
    if(cl_strcmp(cmd, "clear") == 0) {
        FormattedLog("[CMD_INTERPRETOR][CLEAR] Clear screen is triggered!!!!\n");
        ClearScreen();
    } else
    if(cl_strcmp(cmd, "time") == 0) {
        FormattedLog("[CMD_INTERPRETOR][TIME] Yet to be implemented\n");
    } else 
    if(cl_strcmp(cmd, "edit") == 0) {
        FormattedLog("[CMD_INTERPRETOR][EDIT]Yet to be implemented\n");
    } else {
        FormattedLog("[CMD_INTERPRETOR][UNK]\'%s\' is an unknown command\n", cmd);
    }
}
