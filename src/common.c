#include "common.h"
#include "vga.h"
#include <stdarg.h>

size_t string_len(const char* text)
{
    size_t len = 0;
    for (; *text != 0; ++text)
        ++len;
    return len;
}


void memory_copy(char* dst, const char* src, size_t count)
{
    while (count--)
        *dst++ = *src++;
}

void memory_set(char* dst, char value, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        dst[i] = value;
}

void print(const char* format, ...)
{
    // Define number variables here, because we cannot do it inside the switch instruction.
    u64 number_u64;
    i64 number_i64;

    const char gray_on_black = 0x07;

    static Vga_buffer vga =
    {
        .row = 0,
        .col = 0,
        .color = gray_on_black,
    };

    va_list args;
    va_start(args, format);

    for (; *format != 0; ++format)
    {
        if (*format == '%')
        {
            ++format;

            switch (*format)
            {
                case 'i':
                case 'd':
                    number_i64 = va_arg(args, i64);
                    write_dec_signed(&vga, number_i64);
                    break;
                    
                case 'u':
                    number_u64 = va_arg(args, u64);
                    write_dec_unsigned(&vga, number_u64);
                    break;

                case 'x':
                    number_u64 = va_arg(args, u64);
                    write_hex(&vga, number_u64, false, true);
                    break;

                case 'X':
                    number_u64 = va_arg(args, u64);
                    write_hex(&vga, number_u64, true, true);
                    break;

                case 's':
                    char* str = va_arg(args, char*);
                    write_string(&vga, str);
                    break;

                case 'c':
                    write_char(&vga, (char)va_arg(args, int));
                    break;

                case 'z': // color switch
                    vga.color = (char)va_arg(args, int);
                    break;
                
                default:
                    write_char(&vga, '%');
                    --format;
                    break;
            }
        }
        else
        {
            char c = *format;
            write_char(&vga, c);
        }
    }

    va_end(args);

    vga.color = gray_on_black;
}
