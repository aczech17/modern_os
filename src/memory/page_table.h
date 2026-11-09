#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "../common.h"
#include "phys_memory_map.h"

typedef u64 Page_table_entry;

typedef struct
{
    Page_table_entry entry[512];
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
#define PTE_4_KIB_PHYSICAL_ADDRESS_MASK 0x000FFFFFFFFFF000ULL
#define PTE_2_MIB_PHYSICAL_ADDRESS_MASK 0x000FFFFFFFE00000ULL
#define PTE_1_GIB_PHYSICAL_ADDRESS_MASK 0x000FFFFFC0000000ULL

void setup_page_tables
(
    Page_table* pt_level_1,
    Page_table* pt_level_2,
    Page_table* pt_level_3,
    Page_table pt_level_4[512],
    Phys_memory_map* kernel_regions
);
Phys_addr get_phys_addr(const Page_table* pt_root, const Virt_addr virt_addr);

#endif // PAGE_TABLE_H
