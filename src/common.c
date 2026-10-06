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

void memory_set(u8* dst, char value, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        dst[i] = value;
}

void vga_printf(const char* format, ...)
{
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
        if (*format != '%')
        {
            char c = *format;
            write_char(&vga, c);
            continue;
        }

        // Current *format is %, so increment
        ++format;
        // now *format is the actual specificator.

        switch (*format)
        {
            case 'i':
            case 'd':
            {
                i64 number = va_arg(args, i64);
                write_dec_signed(&vga, number);
                break;
            }
                    
            case 'u':
            {
                u64 number = va_arg(args, u64);
                write_dec_unsigned(&vga, number);
                break;
            }

            case 'x':
            {
                u64 number = va_arg(args, u64);
                write_hex(&vga, number, false, true);
                break;
            }

            case 'X':
            {
                u64 number = va_arg(args, u64);
                write_hex(&vga, number, true, true);
                break;
            }

            case 's':
            {
                char* str = va_arg(args, char*);
                write_string(&vga, str);
                break;
            }

            case 'c':
                write_char(&vga, (char)va_arg(args, int));
                break;

            case 'Z': // color set
                vga.color = (char)va_arg(args, int);
                break;
            case 'z': // color reset
                vga.color = gray_on_black;
                break;
                
            default:
                write_char(&vga, '%');
                --format;
                break;
        }
    }

    va_end(args);
}

void panic(const char* msg)
{
    vga_printf("\nPANIC: %s\n", msg);
    for (;;);
}
