#include <stdint.h>

void _start(void)
{
    *(uint16_t*)0xB8000 = 0x4141;

}
