#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "monitor.h"
#include "descriptor_tables.h"
#include "printf.h"

// Multiboot2 info structure
struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    // Step 1: Initialize descriptor tables (GDT + IDT)
    init_descriptor_tables();

    // Step 2: Print welcome message to screen using monitor
   printf("Hello World!\n");
    printf("Interrupt test: %d\n", 3);

    // Step 3: Trigger software interrupts for testing
    asm volatile("int $0x3");
    asm volatile("int $0x4");

    // Step 4: Placeholder for future multiboot parsing (if needed)
    (void)magic;
    (void)mb_info_addr;

    // Step 5: Loop forever to avoid returning to bootloader
    while (1);

    return 0;
}
