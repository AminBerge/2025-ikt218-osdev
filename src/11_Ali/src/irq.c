#include "monitor.h"
#include "isr.h"

// Called from ASM when an IRQ fires
void irq_handler(registers_t regs) {
    monitor_write("Received IRQ: ");
    monitor_write_dec(regs.int_no);
    monitor_put('\n');
}
