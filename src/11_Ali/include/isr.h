#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// This struct represents the registers pushed by pusha,
// plus the additional values pushed by the processor on interrupt.
typedef struct registers
{
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed automatically by the processor
} registers_t;

// Main handler function
void isr_handler(registers_t regs);

// External ASM ISR declarations
extern void isr0();
extern void isr1();
extern void isr2();
#endif
