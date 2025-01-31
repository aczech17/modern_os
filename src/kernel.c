#include <stdint.h>

void _start(uint64_t kernel_address)
{
    const char* msg = "Booting Modern OS...";
    char* src = (char*)msg;
    char* dst = (char*)0xB8000;
    const char* vga_end = (char*)0xB8FFF;

    for (; *src != 0; src++, dst+=2)
    {
        *dst = *src;
        *(dst + 1) = 0x07;
    }

    // Clear the screen.
    for (; dst < vga_end; dst += 2)
    {
        *dst = ' ';
        *(dst + 1) = 0x07;
    }

    for (;;);
}
