#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "../common.h"
#include <stdbool.h>

typedef struct
{
    u8 present:         1;
    u8 read_write:      1;
    u8 user_supervisor: 1;
    u8 write_through:   1;
    u8 cache_disable:   1;
    u8 accessed:        1;
    u8 dirty:           1;
    u8 page_attr_table: 1;
    u8 page_size:       1;
    u8 global:          1;
    u16 avl:            10;
    u64 phys_addr:      40;
    u8 protection_key:  4;
    u8 execute_disable: 1;

    bool lowest;
}Page_table_entry;

#endif // PAGE_TABLE_H
