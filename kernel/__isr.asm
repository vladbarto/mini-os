segment .text
[BITS 64]
IMPORTFROMC InterruptCommonHandler

exception_handler:
    ; salvati tot ce trebuie (registrii de uz general, de segment si de stare (RFLAGS)
    push RAX
    push RCX
    push RDX
    push RBX
    push RSI
    push RDI
    push RBP
    push RSP
    ; push RIP ; trebuie?
    push R8
    push R9
    push R10
    push R11
    push R12
    push R13
    push R14
    push R15
    push CS
    push DS
    push ES
    push SS
    push FS
    push GS
    push RFLAGS

    call InterruptCommonHandler

    ; before calling iretq you must clear ErrorCode from the stack
    ; i.e. move RSP after ErrorCode
    iretq

%macro isr_err_stub 1 ; declare macro with 1 parameter
global isr_err_stub_%+%1  ; declare a global symbol named isr_stub_{param1}
isr_err_stub_%+%1:
    push 1 ; there is an error code
    push %1 ; interrupt index (first macro parameter)
    jmp exception_handler
%endmacro

%macro isr_no_err_stub 1 ; declare macro with 1 parameter
global isr_no_err_stub_%+%1  ; declare a global symbol named isr_stub_{param1}
isr_no_err_stub_%+%1:
    push 0 ; there is no error code (TODO: check against documentation)
    push %1 ; interrupt index (first macro parameter)
    jmp exception_handler
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

extern exception_handler

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
; FIXME: daca entry-ul ii no_err_stub
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep
