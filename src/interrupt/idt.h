#ifndef INTERRUPT_IDT_H
#define INTERRUPT_IDT_H

#include "../common.h"

typedef struct
{
    u16 offset_low;         // handler address (bits 0-15)
    u16 segment_selector;   // probably always 0x8
    u8 ist;                 // interrupt stack options, probably 0 for now

    u8 type_attrs;
    // 7 - present; 6,5 - requested privilege level; 4 - unused; 3-0 - type (1110 for 64-bit interrupt gate)

    u16 offset_middle;      // handler address (bits 16-31)
    u32 offset_high;        // handler address (bits 32-63)
    u32 reserved;           // must be 0
}Idt_entry;



#endif // INTERRUPT_IDT_H
