import os
import subprocess
import sys

bootloader_sources = ['src/boot/stage1.asm', 'src/boot/stage2.asm']
kernel_sources = ['src/kernel.c', 'src/vga.c', 'src/common.c', 'src/memory/memory_map.c', 'src/memory/allocator.c']
linker_script = 'linker.ld'

def assemble_asm(source, output):
    print(f"Assembling {source}.")
    subprocess.run(['nasm', source, '-o', output], check=True)

def compile_kernel(source_files, output, linker_script):
    print(f"Compiling {', '.join(source_files)} with linker script {linker_script} to {output}.")
    subprocess.run([
    'gcc', '-ffreestanding', '-nostartfiles', '-mno-red-zone', '-Wall', '-Wextra', '-s',
    *source_files, '-T', linker_script, '-e', 'kernel_main', '-o', output
    ], check=True)

def calculate_sectors(file_paths):
    all_file_sizes = sum(os.path.getsize(f) for f in file_paths if os.path.isfile(f))
    print(f"Stage 2 and kernel size is {all_file_sizes} B.")
    sectors = (all_file_sizes + 511) // 512
    print(f"That adds up to {sectors} sectors.")
    with open('out/sectors.inc', 'w') as f:
        f.write(f"SECTORS_TO_READ equ {sectors}\n")
    print("SECTORS_TO_READ written to out/sectors.inc")

def create_disk_image():
    print("Joining the OS files...")
    with open("out/os.img", "wb") as img_file:
        for filename in ["out/stage1.bin", "out/stage2.bin", "out/kernel.elf"]:
            with open(filename, "rb") as f:
                img_file.write(f.read())

def run_qemu(memory_size):
    print(f"Running with {memory_size}B of RAM...")
    subprocess.run(['qemu-system-x86_64', '-drive', 'format=raw,file=out/os.img', '-m', memory_size])

def clean_all():
    subprocess.run(['rm', '-rf', 'out'])

def print_success_message():
    if sys.platform.startswith("win"):
        # Printing green text on Windows is not so easy, so print it normally.
        print('Done\n')
    else:
        # If it's Linux, print green.
        print('\033[32mDone\033[0m\n')

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "clean":
        clean_all()
        return

    if sys.platform.startswith("win"):
        print("WARNING: Compiling this code on Windows may be a failure. You probably need a cross-compiler " +
        "to compile the code to ELF format.")
        input("Press any key to compile though.")
        print()

    os.makedirs('out', exist_ok=True)

    assemble_asm(bootloader_sources[1], 'out/stage2.bin')
    compile_kernel(kernel_sources, 'out/kernel.elf', linker_script)

    calculate_sectors(['out/stage2.bin', 'out/kernel.elf'])

    assemble_asm(bootloader_sources[0], 'out/stage1.bin')
    
    create_disk_image()
    print_success_message()

    if len(sys.argv) > 1 and sys.argv[1] == "run":
        memory_size = sys.argv[2] if len(sys.argv) > 2 else '4G'
        run_qemu(memory_size)

if __name__ == '__main__':
    main()
