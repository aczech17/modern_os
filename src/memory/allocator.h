#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H

#include "memory_map.h"

struct Memory_allocator;
/*
    Memory_map* available_sections;
    Memory_map* kernel_sections;
    char frame_bitmap[FRAME_BITMAP_SIZE];
*/

void init_memory_allocator
    (struct Memory_allocator* allocator, Memory_map* available_sections, Memory_map* kernel_sections);


    
#endif // MEM_ALLOCATOR_H
