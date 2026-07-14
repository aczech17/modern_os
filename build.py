import os
import subprocess
import sys
import shutil
import time

build_tools = ["nasm", "gcc"]
emulation_tools = ["qemu-system-x86_64"]

bootloader_sources = ['src/boot/stage1.asm', 'src/boot/stage2.asm']
kernel_sources = ['src/kernel.c', 'src/vga.c', 'src/common.c', 'src/memory/memory_map.c', 'src/memory/frame_allocator.c',
                  'src/memory/page_table.c']

linker_script_template = 'linker_template.ld'
mem_layout_path = 'out/mem_layout.inc'

stack_bottom = 1 << 20
stack_size = 1 << 24
stack_top = stack_bottom + stack_size
text_addr = stack_top

def check_tools(needed_tools):
    missing_tools = []

    for tool in needed_tools:
        if not shutil.which(tool):
            missing_tools.append(tool);

    if missing_tools:
        print("\033[33mFollowing tools are not installed. Install them first.")
        for tool in missing_tools:
            print(f"{tool} ", end='')
        print("\033[0m\n")
        return False

    return True


def prepare_linker_script():
    print("Preparing linker script...")
    with open(linker_script_template, 'r') as f:
        linker_script_content = f.read()

    linker_script_content = linker_script_content.replace("STACK_BOTTOM", f"0x{stack_bottom:X}")
    linker_script_content = linker_script_content.replace("STACK_SIZE", f"0x{stack_size:X}")
    linker_script_content = linker_script_content.replace("TEXT_ADDR", f"0x{text_addr:X}")

    linker_script_path = 'out/linker.ld'

    with open(linker_script_path, 'w') as f:
        f.write(linker_script_content)
    
    return linker_script_path




def compile_kernel(source_files, output, linker_script_path):
    print(f"Compiling {', '.join(source_files)} with linker script {linker_script_path} to {output}.")

    compile_command = """
    gcc
    -ffreestanding
    -fno-pie
    -fno-pic
    -fno-stack-protector
    -fno-asynchronous-unwind-tables
    -mno-red-zone
    -nostdlib
    -static
    -Wall
    -Wextra
    -s
    {sources}
    -T {linker_script_path}
    -Wl,-e,kernel_main
    -o {output}
""".format(
        sources=" ".join(source_files),
        linker_script_path=linker_script_path,
        output=output
    ).split()

    try:
        subprocess.run(compile_command, check=True)
    except subprocess.CalledProcessError:
        print("\033[31mError compiling kernel.\033[0m") # Red color
        sys.exit(1)

     # Pad kernel ELF to full sectors
    size = os.path.getsize(output)
    padding = (-size) % 512

    if padding:
        with open(output, "ab") as f:
            f.write(b"\0" * padding)
    

def write_kernel_layout(kernel_path):
    kernel_size = os.path.getsize(kernel_path)
    kernel_sectors = (kernel_size + 511) // 512

    with open(mem_layout_path, 'w') as f:
        f.write(f"KERNEL_BLOB_SIZE equ {kernel_size}\n")
        f.write(f"KERNEL_BLOB_SECTORS equ {kernel_sectors}\n")
        f.write(f"STACK_BOTTOM equ {stack_bottom}\n")        
        f.write(f"STACK_SIZE equ {stack_size}\n")
        f.write(f"STACK_TOP equ {stack_top}\n")
        f.write(f"KERNEL_TEXT_ADDR equ {text_addr}\n")


def assemble_asm(source, output):
    print(f"Assembling {source}.")
    try:
        subprocess.run(['nasm', '-f', 'bin', source, '-o', output], check=True)
    except subprocess.CalledProcessError:
        print(f"\033[31mError assembling {source}.\033[0m") # Red color
        sys.exit(1)

def write_stage2_sectors_count(stage2_path):
    stage2_size = os.path.getsize(stage2_path)
    stage2_sectors = (stage2_size + 511) // 512

    with open(mem_layout_path, 'a') as f:
        f.write(f"STAGE2_SECTORS equ {stage2_sectors}")


def create_disk_image():
    print("Joining the OS files...")
    with open("out/os.img", "wb") as img_file:
        for filename in ["out/bootloader_stage1.bin", "out/bootloader_stage2.bin", "out/kernel.elf"]:
            with open(filename, "rb") as f:
                img_file.write(f.read())

def qemu_run(memory_size):
    if not memory_size[-1] in "KMG" or not memory_size[:-1].isdigit():
        print("\033[33mInvalid memory size. Use values like '512M', '4G', etc.\033[0m") # yellow
        return

    print(f"Running with {memory_size}B of RAM...")

    try:
        qemu_process = subprocess.Popen(['qemu-system-x86_64', '-drive', 'format=raw,file=out/os.img', '-m', memory_size])
        qemu_process.wait()
    except KeyboardInterrupt:
        qemu_process.terminate()
        print("\033[33m\nSimulation terminated.\033[0m") # yellow
        sys.exit(0)


def qemu_debug(debug_command, delay):
    qemu = subprocess.Popen(
    [
        "qemu-system-x86_64",
        "-drive", "format=raw,file=out/os.img",
        "-monitor", "stdio"
    ],
    stdin=subprocess.PIPE,
    text=True
)

    time.sleep(delay)

    qemu.stdin.write(f"{debug_command}\n")
    qemu.stdin.flush()

    qemu.wait()


def clean_all():
    shutil.rmtree('out', ignore_errors=True)


def main():
    if sys.platform != 'linux':
        print(f"ERROR: Unsupported platform {sys.platform}.")
        print("This code is meant to be compiled on Linux. Cross compiling to ELF not supported yet.")
        return
    
    if len(sys.argv) > 1 and sys.argv[1] == "clean":
        clean_all()
        return

    if check_tools(build_tools) == False:
        return
    
    os.makedirs('out', exist_ok=True)
    linker_script_path = prepare_linker_script()

    compile_kernel(kernel_sources, 'out/kernel.elf', linker_script_path)
    write_kernel_layout('out/kernel.elf')

    assemble_asm(bootloader_sources[1], 'out/bootloader_stage2.bin')
    write_stage2_sectors_count('out/bootloader_stage2.bin')


    assemble_asm(bootloader_sources[0], 'out/bootloader_stage1.bin')
    
    create_disk_image()

    print('\033[32mDone\033[0m\n') # green

    if len(sys.argv) > 1:
        if check_tools(emulation_tools) == False:
            return

        if sys.argv[1] == "run":
            memory_size = sys.argv[2] if len(sys.argv) > 2 else '4G'
            qemu_run(memory_size)
        if sys.argv[1] == "debug":
            debug_command = sys.argv[2] if len(sys.argv) > 2 else 'info registers'
            delay = int(sys.argv[3]) if len(sys.argv) > 3 else 2
            qemu_debug(debug_command, delay)

if __name__ == '__main__':
    main()

