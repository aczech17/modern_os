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

void set_cr3(u64 pt_tree_addr);
void load_idtr(u64 idtr_addr);
void disable_bios_cursor();
void invalidate_tlb_entry(u64 virtual_address);

size_t string_len(const char* text);
void memory_copy(char* dst, const char* src, size_t count);
void* memset(void* dest, int value, size_t count);
void vga_printf(const char* format, ...);
void panic(const char* msg);

#endif // COMMON_H
