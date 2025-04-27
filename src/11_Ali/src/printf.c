// printf.c
#include "monitor.h"
#include "printf.h"

void printf(const char *format, ...) {
    const char *c = format;
    while (*c) {
        if (*c == '%' && *(c + 1) == 's') {
            // Simplified, replace with argument in future
            monitor_write("[string]");
            c += 2;
        } else if (*c == '%' && *(c + 1) == 'd') {
            monitor_write("[int]");
            c += 2;
        } else {
            char buf[2] = {*c, 0};
            monitor_write(buf);
            c++;
        }
    }
}
