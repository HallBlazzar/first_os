FILES=./build/start.o ./build/firstos.asm.o ./build/firstos.o ./build/stdlib.o ./build/stdio.o
INCLUDES=
FLAGS = -g -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ./build/stdlib.elf

./build/start.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.o

./build/firstos.asm.o: ./src/firstos.asm
	nasm -f elf ./src/firstos.asm -o ./build/firstos.asm.o

./build/firstos.o: ./src/firstos.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/firstos.c -o ./build/firstos.o

./build/stdlib.o: ./src/stdlib.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdlib.c -o ./build/stdlib.o

./build/stdio.o: ./src/stdio.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdio.c -o ./build/stdio.o

clean:
	rm -rf ${FILES}
	rm -rf ./build/stdlib.elf
