#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "main.h"

// #define someVariable someValue


#pragma pack(push)
#pragma pack(1)

#pragma pack(pop)

void
InterruptCommonHandler(
    IN BYTE                        InterruptIndex,   // [0x0, 0xFF]
    IN INTERRUPT_STACK_COMPLETE*   StackPointer,      // Pointer to Stack Pointer After Transfer to Handler (Fig 6-9)
    IN BYTE                        ErrorCodeAvailable, // 0 if not available
    IN COMPLETE_PROCESSOR_STATE*   ProcessorState // Pointer to a structure which contains trap context (see above trap frame dump example)
);


#endif // _INTERRUPT_H_