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

static void scroll_down(Vga_buffer* buffer)
{
    memory_copy(VGA_ADDRESS, VGA_ADDRESS + VGA_WIDTH * 2, (VGA_SIZE - VGA_WIDTH * 2));

    // Clear the bottom row.
    for (size_t col = 0; col < VGA_WIDTH; ++col)
    {
        write_char_vga(' ', VGA_HEIGHT - 1, col, buffer->color);
    }

    buffer->row = VGA_HEIGHT - 1;
    buffer->col = 0;
}

void clear_screen(char color)
{
    for (char* dest = VGA_ADDRESS; dest < VGA_ADDRESS + VGA_SIZE; dest += 2)
    {
        *dest = ' ';
        *(dest + 1) = color;
    }
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

void write_hex(Vga_buffer* buffer, u64 number, bool digit_uppercase, bool strip)
{
    if (number == 0 && strip)
    {
        write_string(buffer, "0x0");
        return;
    }

    write_string(buffer, "0x");

    // 16 nibbles
    char digits[17] = {0};
    for (int i = 0; i < 16; ++i)
    {
        int shift = (15 - i) * 4;
        u64 nibble = ((number >> shift) & 0xF);
        char nibble_rep = digit_uppercase ? "0123456789ABCDEF"[nibble] : "0123456789abcdef"[nibble];
        digits[i] = nibble_rep;
    }

    if (strip)
    {
        int i;
        for (i = 0; digits[i] == '0'; ++i); // skip '0' digits

        write_string(buffer, &digits[i]);
        return;
    }

    write_string(buffer, digits);
}

void write_dec(Vga_buffer* buffer, u64 number)
{
    if (number == 0)
    {
        write_string(buffer, "0");
    }

    char digits[20] = {0}; // (2^64 - 1) has 19 digits.

    int i = 0;
    while (number > 0)
    {
        digits[i++] = (number % 10) + '0';
        number /= 10;
    }
    
    // reverse digit buffer
    int left = 0;
    int right = i - 1;
    while (left < right)
    {
        // swap left and right
        char tmp = digits[left];
        digits[left] = digits[right];
        digits[right] = tmp;

        ++left;
        --right;
    }

    write_string(buffer, digits);
}
