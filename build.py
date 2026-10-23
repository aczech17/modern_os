import argparse
import os
import subprocess
import shutil
import sys
import time

build_tools = ["nasm", "gcc"]
emulation_tools = ["qemu-system-x86_64"]

bootloader_sources = ['src/boot/stage1.asm', 'src/boot/stage2.asm']

kernel_asm_sources_and_outputs = [
    ('src/interrupt/stubs.asm', 'out/interrupt.o')
]

kernel_c_sources = ['src/kernel.c', 'src/vga.c', 'src/common.c', 'src/memory/phys_memory_map.c', 'src/memory/frame_allocator.c',
                  'src/memory/page_table.c',
                  'src/interrupt/handlers.c']

linker_script_template = 'linker_template.ld'
os_image_path = "out/modern_os.img"

mem_layout_path = 'out/mem_layout.inc'

stack_limit = 1 << 20
stack_size = 1 << 24
stack_base = stack_limit + stack_size
text_addr = stack_base

def print_green(message):
    print(f"\033[32m{message}\033[0m")

def print_yellow(message):
    print(f"\033[33m{message}\033[0m")

def print_red(message):
    print(f"\033[31m{message}\033[0m")

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--force-build", action="store_true")

    args, remaining = parser.parse_known_args()

    command_parser = argparse.ArgumentParser()
    subparsers = command_parser.add_subparsers(dest="command")

    subparsers.add_parser("clean")

    run_parser = subparsers.add_parser("run")
    run_parser.add_argument("memory_size", nargs="?", default="4G")

    debug_parser = subparsers.add_parser("debug")
    debug_parser.add_argument(
        "debug_command",
        nargs="?",
        default="info registers"
    )
    debug_parser.add_argument("delay", nargs="?", type=int, default=2)

    command_args = command_parser.parse_args(remaining)
    command_args.force_build = args.force_build

    return command_args

def check_tools(needed_tools):
    missing_tools = []

    for tool in needed_tools:
        if not shutil.which(tool):
            missing_tools.append(tool)

    if missing_tools:
        print_red("Following tools are not installed. Install them first.")
        print_red(" ".join(missing_tools))
        return False

    return True


def prepare_linker_script():
    with open(linker_script_template, 'r') as f:
        linker_script_content = f.read()

    linker_script_content = linker_script_content.replace("TEXT_ADDR", f"0x{text_addr:X}")

    linker_script_path = 'out/linker.ld'

    with open(linker_script_path, 'w') as f:
        f.write(linker_script_content)
    
    return linker_script_path


def assemble_kernel_asm(source, output):
    try:
        subprocess.run(['nasm', '-f', 'elf64', source, '-o', output], check=True)
    except subprocess.CalledProcessError:
        print_red(f"Error assembling kernel source {source}.")
        sys.exit(1)

def compile_kernel(
        c_source_files,
        asm_objects,
        linker_script_path,
        output,
):
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
    {c_source_files}
    {asm_objects}
    -T {linker_script_path}
    -o {output}
""".format(
        c_source_files=" ".join(c_source_files),
        asm_objects=" ".join(asm_objects),
        linker_script_path=linker_script_path,
        output=output
    ).split()

    try:
        subprocess.run(compile_command, check=True)
    except subprocess.CalledProcessError:
        print_red("Error compiling kernel.")
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
        f.write(f"STACK_BASE equ {stack_base}\n")
        f.write(f"STACK_LIMIT equ {stack_limit}\n")
        f.write(f"KERNEL_TEXT_ADDR equ {text_addr}\n")


def assemble_bootloader_stage(source, output):
    try:
        subprocess.run(['nasm', '-f', 'bin', source, '-o', output], check=True)
    except subprocess.CalledProcessError:
        print_red(f"Error assembling {source}.")
        sys.exit(1)

def write_stage2_sectors_count(stage2_path):
    stage2_size = os.path.getsize(stage2_path)
    stage2_sectors = (stage2_size + 511) // 512

    with open(mem_layout_path, 'a') as f:
        f.write(f"STAGE2_SECTORS equ {stage2_sectors}\n")


def create_disk_image():
    with open(os_image_path, "wb") as img_file:
        for filename in ["out/bootloader_stage1.bin", "out/bootloader_stage2.bin", "out/kernel.elf"]:
            with open(filename, "rb") as f:
                img_file.write(f.read())

def qemu_run(memory_size):
    suffixes = {
        "K": ("K", "Ki"),
        "M": ("M", "Mi"),
        "G": ("G", "Gi"),
        "Ki": ("K", "Ki"),
        "Mi": ("M", "Mi"),
        "Gi": ("G", "Gi"),
    }

    suffix = next(
        (s for s in ("Ki", "Mi", "Gi", "K", "M", "G")
         if memory_size.endswith(s)),
        None
    )

    if suffix is None or not memory_size[:-len(suffix)].isdigit():
        print_yellow("Invalid memory size. Use values like '512Mi', '4Gi', '4G', etc.")
        return

    number = memory_size[:-len(suffix)]
    qemu_suffix, binary_suffix = suffixes[suffix]

    qemu_memory_size = number + qemu_suffix

    print(f"Running with {number} {binary_suffix}B of RAM...")

    qemu_process = None
    try:
        qemu_process = subprocess.Popen(
            ['qemu-system-x86_64',
             '-drive', f'format=raw,file={os_image_path}',
             '-m', qemu_memory_size])
        qemu_process.wait()
    except KeyboardInterrupt:
        if qemu_process is not None:
            qemu_process.terminate()
            qemu_process.wait()

        print_yellow("Simulation terminated.")
        sys.exit(0)

def qemu_debug(debug_command, delay):
    qemu = subprocess.Popen(
    [
        "qemu-system-x86_64",
        "-drive", f"format=raw,file={os_image_path}",
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


def build_is_needed():
    if not os.path.exists(os_image_path):
        return True

    image_mod_time = os.path.getmtime(os_image_path)

    if os.path.getmtime(linker_script_template) > image_mod_time:
        return True

    for root, _, files in os.walk('src'):
        for filename in files:
            src_path = os.path.join(root, filename)

            if os.path.getmtime(src_path) > image_mod_time:
                return True

    return False

def build():
    print("Compiling to out directory.")
    os.makedirs('out', exist_ok=True)

    print("Preparing linker script... ", end="")
    linker_script_path = prepare_linker_script()
    print(f"Written to {linker_script_path}")

    for (src, output) in kernel_asm_sources_and_outputs:
        print(f"Assembling {src}... ", end="")
        assemble_kernel_asm(src, output)
        print(f"Assembled {output}")

    asm_objects = [path[1] for path in kernel_asm_sources_and_outputs]

    print("Compiling kernel... ", end="")
    compile_kernel(kernel_c_sources, asm_objects , linker_script_path, 'out/kernel.elf')
    print("Compiled to out/kernel.elf")

    write_kernel_layout('out/kernel.elf')

    print("Assembling bootloader stage 2... ", end="")
    assemble_bootloader_stage(bootloader_sources[1], 'out/bootloader_stage2.bin')
    print("Done")

    write_stage2_sectors_count('out/bootloader_stage2.bin')

    print("Assembling bootloader stage 1... ", end="")
    assemble_bootloader_stage(bootloader_sources[0], 'out/bootloader_stage1.bin')
    print("Done")

    print(f"Joining OS files to {os_image_path}...")
    create_disk_image()

    print_green('All done')


def main():
    if sys.platform != 'linux':
        print(f"ERROR: Unsupported platform {sys.platform}.")
        print("This code is meant to be compiled on Linux. Cross compiling to ELF not supported yet.")
        return

    args = parse_arguments()

    if args.command == "clean":
        clean_all()
        return

    build_request = args.force_build or build_is_needed()

    if build_request:
        if not check_tools(build_tools):
            return
        build()

    if not build_request and args.command is None:
        print("Nothing to do.")

    if args.command == "run":
        if not check_tools(emulation_tools):
            return
        qemu_run(args.memory_size)

    elif args.command == "debug":
        if not check_tools(emulation_tools):
            return
        qemu_debug(args.debug_command, args.delay)

if __name__ == '__main__':
    main()

