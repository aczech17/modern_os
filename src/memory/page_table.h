#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "../common.h"
#include <stdbool.h>

typedef struct
{
    u8 present;
    u8 writable;
    u8 user_accessible;
    u8 write_through_caching;
    u8 cache_disable;
    u8 accessed;
    u8 dirty;
    u8 huge_page;
    u8 global;
    u64 available;
    u64 phys_addr;
    u64 no_execute;
}Page_table_entry;

typedef struct
{
    u64 entry[512];
}Page_table;

u64 value(Page_table_entry*);

#endif // PAGE_TABLE_H
