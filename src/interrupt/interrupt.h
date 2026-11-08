#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "idt.h"

void setup_idt(Idt_entry idt_entries[256]);

#endif // INTERRUPT_H
