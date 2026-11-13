#ifndef MEMORY_COMMON_H
#define MEMORY_COMMON_H

#define FRAME_SIZE (1 << 12)        // 4 KiB

typedef u64 Phys_addr;
typedef u64 Virt_addr;
typedef Phys_addr Frame_address;

#define INVALID_ADDR ((u64)(0))

#endif // MEMORY_COMMON_H
