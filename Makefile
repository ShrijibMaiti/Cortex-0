all:
	nasm -f elf32 boot.asm -o boot.o
	gcc -m32 -ffreestanding -c kernel.c -o kernel.o
	gcc -m32 -ffreestanding -c gdt.c -o gdt.o
	gcc -m32 -ffreestanding -c idt.c -o idt.o
	gcc -m32 -ffreestanding -c timer.c -o timer.o
	gcc -m32 -ffreestanding -c keyboard.c -o keyboard.o
	ld -m elf_i386 -T linker.ld -o myos.bin boot.o kernel.o gdt.o idt.o timer.o keyboard.o
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	echo 'menuentry "MyOS" { multiboot /boot/myos.bin }' > isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir
