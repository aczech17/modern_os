#include "handlers.h"
#include "../common.h"
#include "../memory/frame_allocator.h"
#include "../memory/page_table.h"

void page_fault_handler(u64 error_code, Virt_addr accessed_addr)
{
	vga_printf("\nPage fault handler:\n");
	vga_printf("Error code: %X\n", error_code);
	vga_printf("Accessed address: %X\n", accessed_addr);

	u64 new_frame = allocate_general_frame(&global_frame_allocator);
	if (new_frame == INVALID_ADDR)
	{
		panic("Frame allocation error. Run out of memory?");
	}
	map_address(accessed_addr, new_frame);

	vga_printf("VA %X mapped to %X\n", accessed_addr, new_frame);

	// vga_printf("Don't know what to do...%Z_", 0x87);
	//
	// for (;;);
}
