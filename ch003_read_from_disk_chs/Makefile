all:
	nasm -f bin ./boot.asm -o ./boot.bin # first sector(512 bytes)
	dd if=./message.txt >> ./boot.bin # add message to second sector
	dd if=/dev/zero bs=512 count=1 >> ./boot.bin # fill the second sector with 0 until it's length is 512 bytes
