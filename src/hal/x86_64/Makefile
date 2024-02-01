include ${ROOT_PATH}/common.mk

BUILD:=${ROOT_PATH}/build
BIN:=${ROOT_PATH}/bin
INCLUDE:=-I${ROOT_PATH}/include

all: ${BIN}/kernel.pkg

${BIN}/kernel.pkg: ${BUILD}/OSTool ${BIN}/kernel.bin
	${BUILD}/OSTool ${BIN}/kernel.bin $@
	chmod 777 $@

${BIN}/kernel.bin: ${BUILD}/kernel.o
	objcopy -O binary $< $@

${BUILD}/kernel.o:${BUILD}/kernel_entry.o ${BUILD}/kernel_c.o \
				  ${BUILD}/halstart.o
	ld $^ -o $@ -T ${ROOT_PATH}/linker.ld -Map ${BUILD}/kernel.map

${BUILD}/kernel_entry.o: ./kernel_entry.asm
	nasm -f elf64 -g $< -o $@

${BUILD}/kernel_c.o: ./kernel.c
	gcc ${CFLAGS_X64} ${INCLUDE} -g -c $< -o $@

${BUILD}/halstart.o: ./halstart.c
	gcc ${CFLAGS_X64} ${INCLUDE} -g -c $< -o $@


${BUILD}/OSTool: ${ROOT_PATH}/tool/main.c
	gcc $< -o $@
	chmod 777 $<