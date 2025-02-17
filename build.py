import os
import subprocess
import sys
import shutil

needed_tools = ["nasm", "gcc", "qemu-system-x86_64"]
bootloader_sources = ['src/boot/stage1.asm', 'src/boot/stage2.asm']
kernel_sources = ['src/kernel.c', 'src/vga.c', 'src/common.c', 'src/memory/memory_map.c', 'src/memory/allocator.c']
linker_script = 'linker.ld'

def check_tools():
    for tool in needed_tools:
        if not shutil.which(tool):
            print(f"\033[33m{tool} not found. Make sure it's installed.\033[0m") # yellow
            sys.exit(1)

def assemble_asm(source, output):
    print(f"Assembling {source}.")
    try:
        subprocess.run(['nasm', source, '-o', output], check=True)
    except subprocess.CalledProcessError:
        print(f"\033[31mError assembling {source}.\033[0m") # Red color
        sys.exit(1)

def compile_kernel(source_files, output, linker_script):
    print(f"Compiling {', '.join(source_files)} with linker script {linker_script} to {output}.")
    compile_command = """
        gcc -ffreestanding -nostartfiles -mno-red-zone -Wall -Wextra -s
        {sources} -T {linker} -e kernel_main -o {output}
    """.format(sources=" ".join(source_files), linker=linker_script, output=output).split()

    try:
        subprocess.run(compile_command, check=True)
    except subprocess.CalledProcessError:
        print("\033[31mError compiling kernel.\033[0m") # Red color
        sys.exit(1)

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
    if not memory_size[-1] in "KMG" or not memory_size[:-1].isdigit():
        print("\033[33mInvalid memory size. Use values like '512M', '4G', etc.\033[0m") # yellow
        return

    print(f"Running with {memory_size}B of RAM...")
    subprocess.run(['qemu-system-x86_64', '-drive', 'format=raw,file=out/os.img', '-m', memory_size])

def clean_all():
    subprocess.run(['rm', '-rf', 'out'])

def main():
    if sys.platform.startswith("win"):
        print("ERROR: This code is meant to be compiled on Linux.")
        return
    
    if len(sys.argv) > 1 and sys.argv[1] == "clean":
        clean_all()
        return

    check_tools()
    os.makedirs('out', exist_ok=True)

    assemble_asm(bootloader_sources[1], 'out/stage2.bin')
    compile_kernel(kernel_sources, 'out/kernel.elf', linker_script)

    calculate_sectors(['out/stage2.bin', 'out/kernel.elf'])

    assemble_asm(bootloader_sources[0], 'out/stage1.bin')
    
    create_disk_image()

    print('\033[32mDone\033[0m\n') # green

    if len(sys.argv) > 1 and sys.argv[1] == "run":
        memory_size = sys.argv[2] if len(sys.argv) > 2 else '4G'
        run_qemu(memory_size)

if __name__ == '__main__':
    main()
