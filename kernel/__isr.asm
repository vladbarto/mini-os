segment .data
COMPLETE_PROCESSOR_STATE_OFFSET equ 136     ; count 17*8 bytes
INTERRUPT_INDEX_OFFSET          equ 8       ; count  1*8 bytes
ERROR_CODE_AVAILABLE_OFFSET     equ 8       ; count  1*8 bytes


segment .text
[BITS 64]
extern InterruptCommonHandler

exception_handler:
    ; salvati tot ce trebuie (registrii de uz general, de segment si de stare (RFLAGS)
    ;------------------------------
    ; Prepair COMPLETE_PROCESSOR_STATE (param 4 of InterruptCommonHandler)
    push RAX
    push RCX
    push RDX
    push RBX
    push RSI
    push RDI
    push RBP
    push R8
    push R9
    push R10
    push R11
    push R12
    push R13
    push R14
    push R15
    push FS
    push GS
    ; COMPLETE PROCESSOR_STATE (end param 4)
    ;------------------------------
    ; arg1 = rcx
    ; arg2 = rdx
    ; arg3 = r8
    ; arg4 = r9
    lea R9, [RSP] ; get Complete_Processor_State parameter
    mov R8, [RSP + COMPLETE_PROCESSOR_STATE_OFFSET] ; get InterruptIndex parameter
    mov RDX, [RSP + COMPLETE_PROCESSOR_STATE_OFFSET + INTERRUPT_INDEX_OFFSET] ; get ErrorCodeAvailable parameter
    lea RCX, [RSP + COMPLETE_PROCESSOR_STATE_OFFSET + INTERRUPT_INDEX_OFFSET + ERROR_CODE_AVAILABLE_OFFSET] ; save StackPointer

    ; align stack to 16 bytes
    sub rsp, 8     ; now RSP % 16 == 0
    call InterruptCommonHandler
    add rsp, 8     ; restore to previous RSP value

    ; before calling iretq you must clear ErrorCode from the stack
    ; i.e. move RSP after ErrorCode

    ; restore processor state
    pop GS
    pop FS
    pop R15
    pop R14
    pop R13
    pop R12
    pop R11
    pop R10
    pop R9
    pop R8
    pop RBP
    pop RDI
    pop RSI
    pop RBX
    pop RDX
    pop RCX
    pop RAX
    
    ; Clear the stack
    add RSP, 24 ; FIXME: 16 sau 8? stiva tre sa fie aliniata la 16 bytes
    CLI
    HLT

    iretq

%macro isr_err_stub 1 ; declare macro with 1 parameter
global isr_err_stub_%+%1  ; declare a global symbol named isr_stub_{param1}
isr_err_stub_%+%1:
    push 1 ; there is an error code // ErrorCodeAvailable (param 3)
    push %1 ; interrupt index (first macro parameter) // InterruptIndex (param 2)
    jmp exception_handler
%endmacro

%macro isr_no_err_stub 1 ; declare macro with 1 parameter
global isr_no_err_stub_%+%1  ; declare a global symbol named isr_stub_{param1}
isr_no_err_stub_%+%1:
    push 0x50 ; whatever pseudo errorcode 
    push 0 ; there is no error code // ErrorCodeAvailable (param 3)
    push %1 ; interrupt index (first macro parameter) // InterruptIndex (param 2)
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

; global isr_stub_table
; isr_stub_table:
; %assign i 0 
; %rep    32 
;     dq isr_stub_%+i ; use DQ instead if targeting 64-bit
; %assign i i+1 
; %endrep


global isr_stub_table
isr_stub_table:
    dq isr_no_err_stub_0
    dq isr_no_err_stub_1
    dq isr_no_err_stub_2
    dq isr_no_err_stub_3
    dq isr_no_err_stub_4
    dq isr_no_err_stub_5
    dq isr_no_err_stub_6
    dq isr_no_err_stub_7
    dq isr_err_stub_8
    dq isr_no_err_stub_9
    dq isr_err_stub_10
    dq isr_err_stub_11
    dq isr_err_stub_12
    dq isr_err_stub_13
    dq isr_err_stub_14
    dq isr_no_err_stub_15
    dq isr_no_err_stub_16
    dq isr_err_stub_17
    dq isr_no_err_stub_18
    dq isr_no_err_stub_19
    dq isr_no_err_stub_20
    dq isr_no_err_stub_21
    dq isr_no_err_stub_22
    dq isr_no_err_stub_23
    dq isr_no_err_stub_24
    dq isr_no_err_stub_25
    dq isr_no_err_stub_26
    dq isr_no_err_stub_27
    dq isr_no_err_stub_28
    dq isr_no_err_stub_29
    dq isr_err_stub_30
    dq isr_no_err_stub_31
