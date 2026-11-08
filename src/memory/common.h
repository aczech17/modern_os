#ifndef MEMORY_COMMON_H
#define MEMORY_COMMON_H

#define FRAME_SIZE (1 << 12)        // 4 KiB
#define FRAME_BITMAP_SIZE (1 << 20) // 1 MiB

typedef u64 Phys_addr;
typedef u64 Virt_addr;
typedef Phys_addr Frame_address;

/*
    max RAM size = 32 GiB
    frame size = 4 KiB
    max frame count = 32 GiB / 4 KiB = 8 Mi frames
    8 Mi frames -> 8 Mi bools -> 8 Mib = 1 MiB
*/

#define INVALID_ADDR ((u64)(0))

#endif // MEMORY_COMMON_H
