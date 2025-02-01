#include "common.h"

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
