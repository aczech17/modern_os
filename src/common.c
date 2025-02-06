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
    for (size_t i = 0; i < count; ++i)
        dst[i] = src[i];
}

void memory_set(char* dst, char value, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        dst[i] = value;
}

void print(const char* format, ...)
{
    u64 number; // Define it here, because switch is dumb.
    const char gray_on_black = 0x07;

    static Vga_buffer vga =
    {
        .row = 0,
        .col = 0,
        .color = gray_on_black,
    };

    va_list args;
    va_start(args, format);

    while (*format != 0)
    {
        if (*format == '%')
        {
            ++format;

            switch (*format)
            {
                case 'd':
                case 'i':
                    number = va_arg(args, u64);
                    write_dec(&vga, number);
                    break;

                case 's':
                    char* str = va_arg(args, char*);
                    write_string(&vga, str);
                    break;

                case 'x':
                    number = va_arg(args, u64);
                    write_hex(&vga, number, false, true);
                    break;
                case 'X':
                    number = va_arg(args, u64);
                    write_hex(&vga, number, true, true);
                    break;

                case 'z': // color switch
                    vga.color = (char)va_arg(args, int);
                    break;
                
                default:
                    break;
            }
        }
        else
        {
            char str[2] = {0, 0};
            str[0] = *format;
            write_string(&vga, str);
        }

        ++format;
    }

    va_end(args);

    vga.color = gray_on_black;
}
