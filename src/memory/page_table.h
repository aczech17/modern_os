#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "../common.h"
#include "memory_map.h"

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

typedef struct
{
    Page_table tables[4];
}Page_table_tree;

void zero_page_table_tree(Page_table_tree* tree);
void identity_map_kernel(Page_table_tree* tree, const Memory_map* kernel_regions);
u64 get_phys_addr(const Page_table_tree* tree, u64 virt_addr);

#endif // PAGE_TABLE_H
