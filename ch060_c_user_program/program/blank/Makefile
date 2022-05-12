FILES=./build/blank.o
INCLUDES=

all: ${FILES}
	i686-elf-gcc -g -T ./linker.ld -o ./build/blank.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES} ../stdlib/build/stdlib.elf

./build/blank.o: ./blank.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -I ./ -std=gnu99 -c ./blank.c -o ./build/blank.o

clean:
	rm -rf ${FILES}
	rm -rf ./build/blank.elf
