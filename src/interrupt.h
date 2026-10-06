#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "common.h"

typedef struct
{
    u16 offset_low;         // handler address (bits 0-15)
    u16 segment_selector;   // probably always 0x8
    u8 ist;                 // interrupt stack options, probably always 0 for now
    u8 type_attrs;          // 
    u16 offset_middle;      // handler address (bits 16-31)
    u32 offset_high;        // handler address (bits 32-63)
    u32 reserved;           // must be 0
}Idt_entry;

// Interrupt handlers require some assembly stubs.
void page_fault_handler(void);

#endif // INTERRUPT_H
