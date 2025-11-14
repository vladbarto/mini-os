#include "interrupt.h"

__attribute__((aligned(16)))
idt_entry_t idt[256];

idtr_t idtr;

volatile uint64_t g_TickCount = 0;
static uint8_t kbd_ext_state = 0; // 0 = none, 0xE0 or 0xE1 if prefix seen
BOOL EditMode = false;

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

unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;
enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};
int __get_update_in_progress_flag();
unsigned char __get_RTC_register(int reg);
void __read_rtc();
void __InterpretCommand(char* cmd);
void __InterpretKey(WORD ch);
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
            WORD ch = 0x0000;
            if (kbd_ext_state) {
                ch = _kkybrd_scancode_ext[sc_masked];
                kbd_ext_state = 0;
            } else {
                ch = _kkybrd_scancode_std[sc_masked];
            }

            if (!released && ch) {
                __InterpretKey(ch);
            }
       }
    }
}

void __InterpretCommand(char* cmd) {
    FormattedLog("\n[CMD_INTERPRETOR] COMMAND = %s\n", cmd);
    if(cl_strcmp(cmd, "clear") == 0) {
        FormattedLog("[CMD_INTERPRETOR][CLEAR] Clear screen is triggered!!!!\n");
        ClearScreen();
        EnterNewLine(1);
    } else
    if(cl_strcmp(cmd, "time") == 0) {
        INT64 ClockTicksPassed = __rdtsc();
        LogSerialAndScreen("\n[CMD_INTERPRETOR][TIME] Passed Clock Ticks since boot = %D\n", ClockTicksPassed);
        __read_rtc();
        EnterNewLine(1);
    } else 
    if(cl_strcmp(cmd, "edit") == 0) {
        FormattedLog("[CMD_INTERPRETOR][EDIT] Edit mode enabled.\n");
        EditMode = true;
        EnterNewLine(1);
        SaveScreen((void*)MainScreenVideoMemoryBuffer, MAX_LINES * MAX_COLUMNS * 2, &MainScreenCursorPosition);
        if (!EditScreenInitialized) {
            ClearScreen();
            LogSerialAndScreen("Edit Mode\n");
            EditScreenInitialized = true;
        } else {
            RestoreScreen((void*)EditScreenVideoMemoryBuffer, MAX_LINES * MAX_COLUMNS * 2, &EditScreenCursorPosition);
        }
    } else {
        LogSerialAndScreen("\n[CMD_INTERPRETOR][UNK] \'%s\' is an unknown command\n", cmd);
        EnterNewLine(1);
    }
}

void __InterpretKey(WORD ch) {
    // extended char
    if (ch == '\n' || ch == '\r' || ch == KEY_KP_ENTER)
    {
        if (!EditMode) {
            __InterpretCommand(BufferCLI);
            cl_flush(BufferCLI, MAX_COLUMNS);
        } else {
            EnterNewLine(0);
        }
    } else if (ch == KEY_BACKSPACE) {
        ClearCharacter();
        cl_backspace_buffer(BufferCLI); 
    } else if (EditScreenInitialized && ch == KEY_ESCAPE) {
        if (EditMode) {
            EditMode = false;
            FormattedLog("Edit mode exitted.\n");
            SaveScreen((void*)EditScreenVideoMemoryBuffer, MAX_LINES * MAX_COLUMNS * 2, &EditScreenCursorPosition);
            RestoreScreen((void*)MainScreenVideoMemoryBuffer, MAX_LINES * MAX_COLUMNS * 2, &MainScreenCursorPosition);
        }
    } else if (KEY_UP == ch || KEY_DOWN == ch || KEY_LEFT == ch || KEY_RIGHT == ch) {
        if (EditMode) {
            MoveTextCursor(ch);
        }
    } else if (KEY_CAPSLOCK == ch) {
        CapsLockOn = !CapsLockOn;
    }
    // display printable char
    else {
        BYTE OneByteChar = (BYTE) 0xFF & ch;
        if(CapsLockOn == true) {
            if(OneByteChar >= 'a' && OneByteChar <= 'z')
                OneByteChar -= 'a' - 'A';
        }
        if(!EditMode) {
            // only then do accumulate chars
            size_t len = cl_strlen(BufferCLI);
            if (len < MAX_COLUMNS - 1) {
                BufferCLI[len] = OneByteChar;
                BufferCLI[len + 1] = '\0';
            }
        }
        LogSerialAndScreen("%c", OneByteChar);
    }
}

int __get_update_in_progress_flag() {
      __outbyte(cmos_address, 0x0A);
      return (__inbyte(cmos_data) & 0x80);
}

unsigned char __get_RTC_register(int reg) {
      __outbyte(cmos_address, reg);
      return __inbyte(cmos_data);
}

void __read_rtc(void) {
    unsigned char last_second, last_minute, last_hour, last_day, last_month, last_year;
    unsigned char registerB;

    while (__get_update_in_progress_flag());
    second = __get_RTC_register(0x00);
    minute = __get_RTC_register(0x02);
    hour   = __get_RTC_register(0x04);
    day    = __get_RTC_register(0x07);
    month  = __get_RTC_register(0x08);
    year   = __get_RTC_register(0x09) & 0xFF;
    
    do {
        last_second  = second;
        last_minute  = minute;
        last_hour    = hour;
        last_day     = day;
        last_month   = month;
        last_year    = year;

        while (__get_update_in_progress_flag());
        second = __get_RTC_register(0x00);
        minute = __get_RTC_register(0x02);
        hour   = __get_RTC_register(0x04);
        day    = __get_RTC_register(0x07);
        month  = __get_RTC_register(0x08);
        year   = __get_RTC_register(0x09) & 0xFF;

    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour) ||
             (last_day != day) || (last_month != month) || (last_year != year));

    registerB = __get_RTC_register(0x0B);

    // BCD to binary
    if (!(registerB & 0x04)) {
        second = ((second >> 4) * 10) + (second & 0x0F);
        minute = ((minute >> 4) * 10) + (minute & 0x0F);
        hour   = (((hour >> 4) & 0x07) * 10) + (hour & 0x0F) | (hour & 0x80);
        day    = ((day >> 4) * 10) + (day & 0x0F);
        month  = ((month >> 4) * 10) + (month & 0x0F);
        year   = ((year >> 4) * 10) + (year & 0x0F);
    }

    // Convert to 24h if needed
    if (!(registerB & 0x02) && (hour & 0x80))
        hour = ((hour & 0x7F) + 12) % 24;


    LogSerialAndScreen("%u/%u/%u %u:%c%u:%c%u\n", day, month, year + 2000, hour, (minute<10)?'0':0,minute, (second<10)?'0':0, second);
}
