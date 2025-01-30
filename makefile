default:	build

run:	build
	qemu-system-x86_64 -drive format=raw,file=out/os.img -m 2048

build:	stage1 stage2
	cat out/stage1.o out/stage2.o > out/os.img

# First build the stage 2, so we know sector count of stage 2 to give it to the stage1 assembly code.
stage1:	src/stage1.asm calculate_sectors
	nasm src/stage1.asm -o out/stage1.o

# Calculating stage2 sector count. The output is in out/sectors.inc.
calculate_sectors: stage2 calculate_sectors.sh
	chmod +x calculate_sectors.sh
	./calculate_sectors.sh out/stage2.o

stage2:	src/stage2.asm directories
	nasm src/stage2.asm -o out/stage2.o

directories:
	mkdir -p out

clean:
	rm -rf out
