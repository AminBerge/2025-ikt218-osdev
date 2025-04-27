#include "monitor.h"
#include "libc/stdint.h"

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0F

static uint16_t* video_memory = (uint16_t*) VIDEO_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

void monitor_put(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        video_memory[cursor_y * MAX_COLS + cursor_x] = (WHITE_ON_BLACK << 8) | c;
        cursor_x++;
    }

    if (cursor_x >= MAX_COLS) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= MAX_ROWS) {
        // Scroll screen up
        for (int i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++) {
            video_memory[i] = video_memory[i + MAX_COLS];
        }

        for (int i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_ROWS * MAX_COLS; i++) {
            video_memory[i] = (WHITE_ON_BLACK << 8) | ' ';
        }

        cursor_y = MAX_ROWS - 1;
    }
}

void monitor_write(const char *c) {
    int i = 0;
    while (c[i]) {
        monitor_put(c[i++]);
    }
}

void monitor_write_dec(unsigned int n) {
    char str[11];
    int i = 0;

    if (n == 0) {
        monitor_put('0');
        return;
    }

    while (n > 0) {
        str[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i--) {
        monitor_put(str[i]);
    }
}
