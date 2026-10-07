#ifndef INTERRUPT_HANDLERS_H
#define INTERRUPT_HANDLERS_H

#include "../common.h"

void page_fault_stub();
void page_fault_handler(u64 error_code);

#endif // INTERRUPT_HANDLERS_H
