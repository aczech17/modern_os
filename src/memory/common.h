#ifndef MEMORY_COMMON_H
#define MEMORY_COMMON_H

#define FRAME_SIZE (1 << 12)        // 4 KiB
#define FRAME_BITMAP_SIZE (1 << 20) // 1 MiB
/*
    max RAM size = 32 GiB
    frame size = 4 KiB
    max frame count = 32 GiB / 4 KiB = 8 Mi frames
    8 Mi frames -> 8 Mi bools -> 8 Mib = 1 MiB
*/

#define INVALID_ADDR ((u64)(0))

#endif // MEMORY_COMMON_H
