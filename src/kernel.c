#include "vga.h"

void kernel_main(u64 kernel_address)
{
    clear_screen(0x07);

    print("Super system kurwo!\n");
    print("%zBrawo, Robercik!\n", 0x14);
    print("%zHej, Jaga gol!\n", 0x4E);

    print("Kernel jest pod adresem %X, czyli po ludzku %u.\n\n", kernel_address, kernel_address);

    u64 number = 0xdeadbeef;
    print("Test pisania liczb:\n%x = %X\n", number, number);

    print("procent: %");
    print("\n%M");
    print("\n0\n");
    print("min i64 = %i\n", (i64)0xFFFFFFFFFFFFFFFF);
    print("max u64 = %u\n", 0xFFFFFFFFFFFFFFFF);
    print("Litera: %c\n", 'A');
    print("Procent raz jeszcze: %c", '%');
    print("\nKolor zmieniony: %zHello!\n", 0x1F);
    print("Juz nie\n");

    for (;;);
}
