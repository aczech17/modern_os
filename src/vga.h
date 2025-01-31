#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    size_t row, col;
    char color;
}Vga_buffer;

void clear_screen(Vga_buffer*);
void write_string(Vga_buffer*, const char* text);
void write_hex(Vga_buffer*, uint64_t number, bool strip);

#endif // VGA_H
