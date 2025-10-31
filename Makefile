SRC_DIR = kernel
SRC_C =\
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/main.h \
	$(SRC_DIR)/screen.c \
	$(SRC_DIR)/screen.h \
	$(SRC_DIR)/logging.c \
	$(SRC_DIR)/logging.h \
	$(SRC_DIR)/string.c \
	$(SRC_DIR)/string.h \
	$(SRC_DIR)/bochs_map.h \
	$(SRC_DIR)/ata_commands.h \
	$(SRC_DIR)/scancode.h \
	$(SRC_DIR)/interrupt.c \
	$(SRC_DIR)/interrupt.h
SRC_ASM =\
	$(SRC_DIR)/__init.asm \
	$(SRC_DIR)/__isr.asm

.PHONY: make-floppy kernel init make-floppy32 kernel32 init32

make-floppy: kernel
	python3 utils/makeFloppy.py boot/mbr.asm boot/ssl.asm

kernel: $(SRC_C) init
	x86_64-w64-mingw32-gcc -O0 -ffreestanding -nostdlib \
	-Wl,--image-base,0x200000,-e,ASMEntryPoint,\
	--section-alignment,0x1000,--file-alignment,0x1000,--subsystem,native,\
	-T,link_script.ld \
	kernel/__init.o kernel/__isr.o \
	kernel/main.c kernel/logging.c kernel/screen.c kernel/interrupt.c \
	-o bin/kernel.exe

init: $(SRC_ASM)
	nasm -O0 -fwin64 kernel/__init.asm -o kernel/__init.o
	nasm -O0 -fwin64 kernel/__isr.asm -o kernel/__isr.o

make-floppy32: kernel32
	python3 utils/makeFloppy.py boot/mbr.asm boot/ssl.asm

kernel32: $(SRC_C) init32
	i686-w64-mingw32-gcc -ffreestanding -nostdlib \
	-Wl,--image-base,0x200000,-e,_ASMEntryPoint,\
	--section-alignment,0x1000,--file-alignment,0x1000,--subsystem,native \
	kernel/__init.o kernel/__isr.o \
	kernel/main.c kernel/logging.c kernel/screen.c kernel/interrupt.c \
	-o bin/kernel.exe

init32: $(SRC_ASM)
	nasm -fwin32 kernel/__init.asm -o kernel/__init.o
	nasm -fwin32 kernel/__isr.asm -o kernel/__isr.o
