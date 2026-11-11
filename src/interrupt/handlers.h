#ifndef INTERRUPT_HANDLERS_H
#define INTERRUPT_HANDLERS_H

#include "../common.h"
#include "../memory/common.h"

void page_fault_isr();
void page_fault_handler(u64 error_code, Virt_addr accessed_addr);

#endif // INTERRUPT_HANDLERS_H
