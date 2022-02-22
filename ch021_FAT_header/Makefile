FILES = ./build/kernel.asm.o ./build/kernel.o ./build/disk/disk_stream.o ./build/disk/disk.o ./build/string/string.o ./build/fs/path_parser.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o
INCLUDES = -I ./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin

	# fill os.bin with enough 512 sectors
	# Disk read loads 512 byte a time. If target data sit in sector not 512 byte, it will be ignored
	# Here 16MB is filled. (1MB=1048576 byte) --> Total size of OS. Files and other things(kernel, bootloader, etc.) all placed there
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin

	# Write a sample file to filesystem
	sudo mount -t vfat bin/os.bin /mnt/firstos
	sudo cp ./sample.txt /mnt/firstos
	sudo umount /mnt/firstos

./bin/kernel.bin: $(FILES)
	# link all FILES as a single object file
	# Note that while linking, "./build/kernel.asm.o" should always be first linked one
	# The reason is it contains code to enter protected mode
	# If other code are execute before it, they might not work properly
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o

	# invoke GCC to generate kernel.bin according to linker.ld
	# -O0 -> no optimization
	# -nostdlib -> don't include standard library from host
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/idt -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/memory/memory.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/memory -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/memory/heap -std=gnu99 -c ./src/memory/heap/heap.c -o ./build/memory/heap/heap.o

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/memory/heap -std=gnu99 -c ./src/memory/heap/kheap.c -o ./build/memory/heap/kheap.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/memory/paging -std=gnu99 -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/disk/disk.o: ./src/disk/disk.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/disk -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/disk/disk_stream.o: src/disk/disk_stream.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/disk -std=gnu99 -c ./src/disk/disk_stream.c -o ./build/disk/disk_stream.o

./build/fs/path_parser.o: ./src/fs/path_parser.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/fs -std=gnu99 -c ./src/fs/path_parser.c -o ./build/fs/path_parser.o

./build/string/string.o: ./src/string/string.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/string -std=gnu99 -c ./src/string/string.c -o ./build/string/string.o

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o