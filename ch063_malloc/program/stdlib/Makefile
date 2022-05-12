FILES=./build/start.o ./build/firstos.o ./build/stdlib.o
INCLUDES=
FLAGS= -g

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ./build/stdlib.elf

./build/start.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.o

./build/firstos.o: ./src/firstos.asm
	nasm -f elf ./src/firstos.asm -o ./build/firstos.o

./build/stdlib.o: ./src/stdlib.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdlib.c -o ./build/stdlib.o

clean:
	rm -rf ${FILES}
	rm -rf ./build/stdlib.elf
