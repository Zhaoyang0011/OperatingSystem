include ${ROOT_PATH}/common.mk

BUILD:=${ROOT_PATH}/build
BIN:=${ROOT_PATH}/bin
INCLUDE:=-I${ROOT_PATH}/include

all: ${BIN}/boot.bin ${BIN}/setup.bin

${BIN}/boot.bin: ./boot.asm
	nasm $< -o $@

${BIN}/setup.bin: ${BUILD}/setup.o
	objcopy -O binary $< $@

${BUILD}/setup.o:${BUILD}/setup_asm.o ${BUILD}/setup_c.o
	ld -m elf_i386 $^ -o $@ -Ttext 0x500

${BUILD}/setup_asm.o: ./setup.asm
	nasm -f elf32 -g $< -o $@

${BUILD}/setup_c.o: ./setup.c
	gcc ${CFLAGS_X86} ${INCLUDE} -g -c $< -o $@
