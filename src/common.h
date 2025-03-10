#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

size_t string_len(const char* text);
void memory_copy(char* dst, const char* src, size_t count);
void memory_set(u8* dst, char value, size_t count);
void print(const char* format, ...);
void panic(const char* msg);

#endif // COMMON_H
