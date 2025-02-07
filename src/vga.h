#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stddef.h>
#include "common.h"

typedef struct
{
    size_t row, col;
    char color;
}Vga_buffer;

void clear_screen(char color);
void write_char(Vga_buffer*, const char);
void write_string(Vga_buffer*, const char* text);
void write_hex(Vga_buffer*, u64 number, bool digit_uppercase, bool strip);
void write_dec_unsigned(Vga_buffer* buffer, u64 number);
void write_dec_signed(Vga_buffer* buffer, i64 number);

#endif // VGA_H
