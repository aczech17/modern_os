#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void clear_vga();
void write_char_vga(const char c, size_t row, size_t col);
void write_string_vga(const char* text, size_t row, size_t col);
void write_hex(uint64_t number, size_t row, size_t col, bool truncate);

#endif // VGA_H
