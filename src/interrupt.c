#include "common.h"

void page_fault_handler(u64 error_code)
{
    vga_printf("\n%Zerror code: %X%z", 0x64, error_code);
    panic("Page fault horror show");
}
