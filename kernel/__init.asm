;;-----------------_DEFINITIONS ONLY_-----------------------
;; IMPORT FUNCTIONS FROM C
%macro IMPORTFROMC 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    extern _%1
    %1 equ _%1
    %else
    extern %1
    %endif
%rotate 1 
%endrep
%endmacro

;; EXPORT TO C FUNCTIONS
%macro EXPORT2C 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    global _%1
    _%1 equ %1
    %else
    global %1
    %endif
%rotate 1 
%endrep
%endmacro

%define break xchg bx, bx

IMPORTFROMC KernelMain

TOP_OF_STACK                equ 0x200000
KERNEL_BASE_PHYSICAL        equ 0x200000

section .bss
align 4096
PML4:   resq      512 ;resq
align 4096
PDPT:   resq      512 ;resq
align 4096
PD:     resq      512 ;resq

;;-----------------^DEFINITIONS ONLY^-----------------------

segment .text
[BITS 32]
ASMEntryPoint:
    cli
    MOV     DWORD [0x000B8000], 'O1S1'
%ifidn __OUTPUT_FORMAT__, win32
    MOV     DWORD [0x000B8004], '3121'                  ; 32 bit build marker
%else
    MOV     DWORD [0x000B8004], '6141'                  ; 64 bit build marker
%endif



    MOV     ESP, TOP_OF_STACK                           ; just below the kernel
    
    break

    ;TODO!!! define page tables; see https://wiki.osdev.org ,Intel's manual, http://www.brokenthorn.com/Resources/ ,http://www.jamesmolloy.co.uk/tutorial_html/
    ;PML4 points to PDPT
    MOV EAX, PDPT
    OR EAX, 0x03            ; bit 0 - PDPT present; bit 1 - R/W activated
    MOV [PML4 + 0*8], EAX   ; first entry in PML4
    MOV dword [PML4 + 0*8 + 4], 0

    ;PDPT points to PD
    MOV EAX, PD
    OR EAX, 0x03
    MOV [PDPT + 0*8], EAX
    MOV dword [PDPT + 0*8 + 4], 0

    ;PD points to PT
    MOV EAX, 0x00000000
    OR EAX, 0x83
    MOV [PD + 0*8], EAX
    MOV dword [PD + 0*8 + 4], 0

    ; PD[1] = 0x00200000 | PS | P | RW (2..4MB)
    MOV eax, 0x00200000
    OR  eax, 0x83
    MOV [PD + 1*8], eax
    MOV dword [PD + 1*8 + 4], 0




    ;TODO!!! activate paging
; store base physical address of PML4 in CR3
    MOV EAX, PML4
    MOV CR3, EAX

;4 level paging: 
; CR4.PAE = 1:
    MOV EAX, CR4
    OR EAX, 0x00000020 ; Set PAE bit on 1
    MOV CR4, EAX


; IA32_EFER.LME = 1:
    MOV ECX, 0xC0000080     ; base address of EFER in MSR
    RDMSR                   ; EDX:EAX <-- MSR[ECX] (which is IA32_EFER) 
    OR EAX, 1 << 8             ; 0x00000008 ; Set EFER.LME on 1 (bit 8)
    WRMSR                   ;  MSR[ECX] <-- EDX:EAX;

; ;CR4.LA57 = 0:
    MOV EAX, CR4
    AND EAX, 0xFFFFEFFF     ; Clear CR4.LA57 (bit 12) ; nu cu 0, trebe cu F-uri
    MOV CR4, EAX

; ; CR0.PG = 1:
    MOV EAX, CR0
    OR EAX, 0x80000000 ; Set PG bit on 1
    MOV CR0, EAX


    ;TODO!!! transition to 64bits-long mode
[BITS 64]

    MOV     EAX, KernelMain     ; after 64bits transition is implemented the kernel must be compiled on x64
    CALL    EAX
    
    break
    CLI
    HLT

;;--------------------------------------------------------

__cli:
    CLI
    RET

__sti:
    STI
    RET

__magic:
    XCHG    BX,BX
    RET
    
__enableSSE:                ;; enable SSE instructions (CR4.OSFXSR = 1)  
    MOV     EAX, CR4
    OR      EAX, 0x00000200
    MOV     CR4, EAX
    RET
    
EXPORT2C ASMEntryPoint, __cli, __sti, __magic, __enableSSE


