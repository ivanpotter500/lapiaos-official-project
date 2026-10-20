CC = i686-elf-gcc
LD = i686-elf-ld
CFLAGS = -ffreestanding -O2 -Wall -Wextra

all: kernel.bin

boot.o: boot.asm
	nasm -f elf32 boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kernel.bin: boot.o kernel.o linker.ld
	$(LD) -T linker.ld -o kernel.bin -nostdlib boot.o kernel.o

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin -nographic -serial mon:stdio

iso: kernel.bin
	rm -rf iso
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "LapiaOS" { multiboot /boot/kernel.bin; boot }' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o lapiaos.iso iso/

runiso: iso
	qemu-system-i386 -cdrom lapiaos.iso -boot d -nographic -serial mon:stdio

clean:
	rm -f *.o kernel.bin
	rm -rf iso lapiaos.iso

.PHONY: all run iso runiso clean
