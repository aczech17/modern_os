import os
import subprocess
import sys

def assemble_asm(source, output):
    print(f"Assembling {source}.")
    subprocess.run(['nasm', source, '-o', output], check=True)

def compile_c_nostd(source_files, output):
    print(f"Compiling {', '.join(source_files)} to {output}.")
    subprocess.run(['gcc', '-nostdlib', '-Wall', '-Wextra', *source_files, '-o', output], check=True)
    subprocess.run(['strip', output])

def calculate_sectors(file_paths):
    all_file_sizes = 0
    for file_path in file_paths:
        if not os.path.isfile(file_path):
            print(f"Error: File {file_path} not found!")
            exit(1)
        file_size = os.path.getsize(file_path)
        print(f"File size: {file_size}")
        all_file_sizes += file_size


    sectors = (all_file_sizes + 511) // 512  # Round up to 512.
    print(f"Sectors to read from disk: {sectors}")

    with open('out/sectors.inc', 'w') as f:
        f.write(f"SECTORS_TO_READ equ {sectors}\n")

    print("SECTORS_TO_READ written to out/sectors.inc")

def create_disk_image():
    print("Joining object files...")
    with open("out/os.img", "wb") as img_file:
        # Łączenie stage1.o i stage2.o w jeden plik obrazu
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

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "clean":
        clean_all()
        return

    os.makedirs('out', exist_ok=True)

    # Assemble stage 2
    assemble_asm('src/stage2.asm', 'out/stage2.elf')
    # compile_c_nostd('src/kernel.c', 'out/kernel.elf')
    # compile_c_nostd('src/vga.c', 'out/vga.elf')
    compile_c_nostd(['src/kernel.c', 'src/vga.c'], 'out/kernel.elf')
    calculate_sectors(['out/stage2.elf', 'out/kernel.elf'])

    # Assemble stage 1
    assemble_asm('src/stage1.asm', 'out/stage1.elf')
    create_disk_image()

    if len(sys.argv) > 1 and sys.argv[1] == "run":
        run_qemu()

# Uruchomienie programu
if __name__ == '__main__':
    main()
