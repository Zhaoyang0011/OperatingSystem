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

${BUILD}/kernel.o:${BUILD}/kernel_entry.o ${BUILD}/kernel_main.o \
				${BUILD}/halinit.o ${BUILD}/halglobal.o ${BUILD}/phymem.o \
				${BUILD}/kprint.o ${BUILD}/cpu.o ${BUILD}/cpuctrl.o \
				${BUILD}/apci.o ${BUILD}/apic.o ${BUILD}/mp.o \
				${BUILD}/interrupt.o ${BUILD}/gdtidt.o ${BUILD}/int.asm.o \
				${BUILD}/i8259.o
	ld $^ -o $@ -T ${ROOT_PATH}/linker.ld -Map ${BUILD}/kernel.map

${BUILD}/kernel_entry.o: ./kernel_entry.asm
	nasm -f elf64 -g $< -o $@

${BUILD}/kernel_main.o: ./kernel_main.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/halinit.o: ./halinit.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/halinit.o: ./halinit.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/halglobal.o: ./halglobal.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/kprint.o: ./kprint.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/cpu.o: ./cpu.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/cpuctrl.o: ./cpuctrl.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/apci.o: ./apci.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/mp.o: ./mp.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/apic.o: ./interrupt/apic.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/int.asm.o: ./interrupt/int.asm
	nasm -f elf64 -g -I ./interrupt $< -o $@

${BUILD}/gdtidt.o: ./interrupt/gdtidt.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/interrupt.o: ./interrupt/interrupt.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/i8259.o: ./interrupt/i8259.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/phymem.o: ./mem/phymem.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/OSTool: ${ROOT_PATH}/tool/main.c
	gcc $< -o $@
	chmod 777 $<