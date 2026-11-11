#include "handlers.h"
#include "../common.h"

void page_fault_handler(u64 error_code, Virt_addr accessed_addr)
{
	vga_printf("\nPage fault handler:\n");
	vga_printf("Error code: %X\n", error_code);
	vga_printf("Accessed address: %X\n", accessed_addr);

	vga_printf("Don't know what to do...%Z_", 0x87);

	for (;;);
}
