#include <stdint.h>

void _start(void)
{
    const char* msg = "Booting Modern OS...";
    char* src = (char*)msg;
    char* dst = (char*)0xB8000;

    for (; *src != 0; src++, dst+=2)
    {
        *dst = *src;
        *(dst + 1) = 0x07;
    }

    for (;;);
}
