import os
import subprocess
import sys

bootloader_sources = ['src/boot/stage1.asm', 'src/boot/stage2.asm']
kernel_sources = ['src/kernel.c', 'src/vga.c', 'src/common.c']


def assemble_asm(source, output):
    print(f"Assembling {source}.")
    subprocess.run(['nasm', source, '-o', output], check=True)

def compile_kernel(source_files, output):
    print(f"Compiling {', '.join(source_files)} to {output}.")
    subprocess.run(['gcc', '-nostdlib', '-mno-red-zone', '-Wall', '-Wextra', *source_files, '-e', 'kernel_main', '-o', output], check=True)
    subprocess.run(['strip', output])

def calculate_sectors(file_paths):
    all_file_sizes = 0
    for file_path in file_paths:
        if not os.path.isfile(file_path):
            print(f"Error: File {file_path} not found!")
            exit(1)
        file_size = os.path.getsize(file_path)
        all_file_sizes += file_size

    print(f"Stage 2 and kernel size is {all_file_sizes} B.")
    sectors = (all_file_sizes + 511) // 512  # Round up to 512.
    print(f"That adds up to {sectors} sectors.")

    with open('out/sectors.inc', 'w') as f:
        f.write(f"SECTORS_TO_READ equ {sectors}\n")

    print("SECTORS_TO_READ written to out/sectors.inc")

def create_disk_image():
    print("Joining the ELF files...")
    with open("out/os.img", "wb") as img_file:
        with open("out/stage1.elf", "rb") as stage1:
            img_file.write(stage1.read())
        with open("out/stage2.elf", "rb") as stage2:
            img_file.write(stage2.read())
        with open("out/kernel.elf", "rb") as kernel:
            img_file.write(kernel.read())

def run_qemu():
    subprocess.run(['qemu-system-x86_64', '-drive', 'format=raw,file=out/os.img', '-m', '2048'])

def clean_all():
    subprocess.run(['rm', '-rf', 'out'])

def print_success_message():
    if sys.platform.startswith("win"):
        # Printing green text on Windows is not so easy, so print it normally.
        print('Done')
    else:
        # If it's Linux, print green.
        print('\033[32mDone\033[0m')

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "clean":
        clean_all()
        return

    os.makedirs('out', exist_ok=True)

    # Assemble stage 2 and compile the kernel.
    assemble_asm(bootloader_sources[1], 'out/stage2.elf')
    compile_kernel(kernel_sources, 'out/kernel.elf')

    # Calculate how many sectors do they take.
    calculate_sectors(['out/stage2.elf', 'out/kernel.elf'])

    # Assemble stage 1
    assemble_asm(bootloader_sources[0], 'out/stage1.elf')

    # Join the elf files to the final OS image.
    create_disk_image()

    print_success_message()

    if len(sys.argv) > 1 and sys.argv[1] == "run":
        print('Running...')
        run_qemu()

if __name__ == '__main__':
    main()
