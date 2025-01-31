#include "vga.h"

#define VGA_ADDRESS ((char*)(0xB8000))
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_SIZE (VGA_WIDTH * VGA_HEIGHT * 2)
#define VGA_COLOR 0x07

void clear_vga()
{
    for (char* dest = VGA_ADDRESS; dest < VGA_ADDRESS + VGA_SIZE; dest += 2)
    {
        *dest = ' ';
        *(dest + 1) = VGA_COLOR;
    }
}

void write_char_vga(const char c, size_t row, size_t col)
{
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH)
        return;

    char* dest = VGA_ADDRESS + (row * VGA_WIDTH + col) * 2;
    *dest = c;
    *(dest + 1) = VGA_COLOR;
}

void write_string_vga(const char* text, size_t row, size_t col)
{
    for (char* src = (char*)text; *src != 0; ++src)
    {
        if (col >= VGA_WIDTH)
        {
            ++row;
            col = 0;
            if (row >= VGA_HEIGHT)
                return;
        }

        if (*src == '\n')
        {
            ++row;
            col = 0;
            if (row >= VGA_HEIGHT)
                return;
            continue;
        }

        write_char_vga(*src, row, col);
        ++col;
    }
}


void write_hex(uint64_t number, size_t row, size_t col, bool truncate)
{
    if (number == 0 && truncate)
    {
        write_string_vga("0x0", row, col);
        return;
    }

    write_string_vga("0x", row, col);
    col += 2;

    // 16 nibbles
    for (int i = 15; i >= 0; --i)
    {
        uint64_t nibble = ((number >> (i * 4)) & 0xF);
        if (nibble == 0 && truncate)
            continue;

        truncate = false;
        char nibble_rep = "0123456789ABCDEF"[nibble];
        write_char_vga(nibble_rep, row, col);
        ++col;
    }
}

