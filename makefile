default:	build

run:	build
	qemu-system-x86_64 -drive format=raw,file=out/os.img -m 2048

build:	stage1 stage2
	cat out/boot.o out/stage2.o > out/os.img

stage1:	src/boot.asm calculate_sectors
	nasm src/boot.asm -o out/boot.o

calculate_sectors:	calculate_sectors.sh stage2 directories
	chmod +x calculate_sectors.sh
	./calculate_sectors.sh out/stage2.o

stage2:	src/stage2.asm directories
	nasm src/stage2.asm -o out/stage2.o

directories:
	mkdir -p out

clean:
	rm -rf out
