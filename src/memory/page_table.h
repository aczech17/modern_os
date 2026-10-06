#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "../common.h"
#include "phys_memory_map.h"

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
    Phys_addr phys_addr;
    u64 no_execute;
}Page_table_entry;

typedef struct
{
    u64 entry[512];
}Page_table;

#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_WRITE_THROUGH (1ULL << 3)
#define PTE_CACHE_DISABLE (1ULL << 4)
#define PTE_ACCESSED (1ULL << 5)
#define PTE_DIRTY (1ULL << 6)
#define PTE_HUGE_PAGE (1ULL << 7)
#define PTE_GLOBAL (1ULL << 8)
#define PTE_NO_EXECUTE (1ULL << 63)
#define PTE_PHYSICAL_ADDRESS_MASK 0x000FFFFFFFFFF000ULL

u64 page_table_value(const Page_table_entry* entry);
Phys_addr get_phys_addr(const Page_table* pt_root, Virt_addr virt_addr);

#endif // PAGE_TABLE_H
