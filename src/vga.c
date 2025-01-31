#include "vga.h"

#define VGA_ADDRESS ((char*)(0xB8000))
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_SIZE (VGA_WIDTH * VGA_HEIGHT * 2)

static void write_char_vga(char c, size_t row, size_t col, char color)
{
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH)
        return;

    char* dest = VGA_ADDRESS + (row * VGA_WIDTH + col) * 2;
    *dest = c;
    *(dest + 1) = color;
}

static char get_char_vga(size_t row, size_t col)
{
    char* addr = VGA_ADDRESS + (row * VGA_WIDTH + col) * 2;
    return *addr;
}

static void scroll_down(Vga_buffer* buffer)
{
    for (size_t row = 0; row < VGA_HEIGHT - 1; ++row)
    {
        // copy the row from the bottom
        for (size_t col = 0; col < VGA_WIDTH; ++col)
        {
            char c = get_char_vga(row + 1, col);
            write_char_vga(c, row, col, buffer->color);
        }
    }

    // Clear the bottom row.
    for (size_t col = 0; col < VGA_WIDTH; ++col)
    {
        write_char_vga(' ', VGA_HEIGHT - 1, col, buffer->color);
    }

    buffer->row = VGA_HEIGHT - 1;
}

void clear_screen(Vga_buffer* buffer)
{
    for (char* dest = VGA_ADDRESS; dest < VGA_ADDRESS + VGA_SIZE; dest += 2)
    {
        *dest = ' ';
        *(dest + 1) = buffer->color;
    }
    /*
        Could be a for loop for rows and cols and calling write_char_vga but that would be slower, I think.
    */


    buffer->row = 0;
    buffer->col = 0;
}

void write_string(Vga_buffer* buffer, const char* text)
{
    for (char* src = (char*)text; *src != 0; ++src)
    {
        if (buffer->col >= VGA_WIDTH)
        {
            ++buffer->row;
            buffer->col = 0;

            if (buffer->row >= VGA_HEIGHT)
            {
                scroll_down(buffer);
            }
        }

        if (*src == '\n')
        {
            ++buffer->row;
            buffer->col = 0;

            if (buffer->row >= VGA_HEIGHT)
            {
                scroll_down(buffer);
            }

            continue;
        }

        write_char_vga(*src, buffer->row, buffer->col, buffer->color);
        ++buffer->col;
    }
}

void write_hex(Vga_buffer* buffer, u64 number, bool strip)
{
    if (number == 0 && strip)
    {
        write_string(buffer, "0x0");
        return;
    }

    write_string(buffer, "0x");

    // 16 nibbles
    for (int i = 15; i >= 0; --i)
    {
        u64 nibble = ((number >> (i * 4)) & 0xF);
        if (nibble == 0 && strip)
            continue;

        strip = false;
        char nibble_rep = "0123456789ABCDEF"[nibble];
        write_char_vga(nibble_rep, buffer->row, buffer->col, buffer->color);
        ++buffer->col;
    }
}

