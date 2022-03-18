FILES = ./build/kernel.asm.o ./build/kernel.o ./build/disk/disk_stream.o ./build/task/task.o ./build/disk/disk.o ./build/task/tss.asm.o ./build/task/task.asm.o ./build/string/string.o ./build/task/process.o ./build/fs/fat/fat16.o ./build/fs/file.o ./build/fs/path_parser.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o ./build/gdt/gdt.o ./build/gdt/gdt.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o
INCLUDES = -I ./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin ./bin/kernel.bin user_program
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

./build/gdt/gdt.o: ./src/gdt/gdt.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/gdt -std=gnu99 -c ./src/gdt/gdt.c -o ./build/gdt/gdt.o

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/gdt/gdt.asm.o

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

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/fs -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o

./build/fs/file.o: ./src/fs/file.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/fs -std=gnu99 -c ./src/fs/file.c -o ./build/fs/file.o

./build/fs/path_parser.o: ./src/fs/path_parser.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/fs -std=gnu99 -c ./src/fs/path_parser.c -o ./build/fs/path_parser.o

./build/string/string.o: ./src/string/string.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/string -std=gnu99 -c ./src/string/string.c -o ./build/string/string.o

./build/task/tss.asm.o: ./src/task/tss.asm
	nasm -f elf -g ./src/task/tss.asm -o ./build/task/tss.asm.o

./build/task/task.o: ./src/task/task.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/task -std=gnu99 -c ./src/task/task.c -o ./build/task/task.o

./build/task/process.o: ./src/task/process.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -I ./src/task -std=gnu99 -c ./src/task/process.c -o ./build/task/process.o

./build/task/task.asm.o: ./src/task/task.asm
	nasm -f elf -g ./src/task/task.asm -o ./build/task/task.asm.o

user_program:
	cd ./program/blank && $(MAKE) all

clean_user_program:
	cd ./program/blank && $(MAKE) clean

clean: clean_user_program
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ${FILES}
	rm -rf ./build/kernelfull.o