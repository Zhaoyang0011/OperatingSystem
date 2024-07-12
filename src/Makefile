include ${ROOT_PATH}/common.mk

SRC:=${ROOT_PATH}/src
BUILD:=${ROOT_PATH}/build
BIN:=${ROOT_PATH}/bin
INCLUDE:=-I${ROOT_PATH}/include

all: ${BIN}/kernel.pkg

${BIN}/kernel.pkg: ${BUILD}/OSTool ${BIN}/kernel.bin
	${BUILD}/OSTool ${BIN}/kernel.bin $@
	chmod 777 $@

${BIN}/kernel.bin: ${BUILD}/kernel.o
	objcopy -O binary $< $@

${BUILD}/kernel.o: ${BUILD}/hal.o  ${BUILD}/kernel_main.o ${BUILD}/krlglobal.o \
				   ${BUILD}/krlinit.o ${BUILD}/virmem.o ${BUILD}/krlmempool.o \
				   ${BUILD}/thread.o ${BUILD}/scheduler.o
	ld $^ -o $@ -T ${ROOT_PATH}/linker.ld -Map ${BUILD}/kernel.map

${BUILD}/hal.o:
	$(MAKE) -C ${SRC}/hal/${ARC} ROOT_PATH=${ROOT_PATH}

${BUILD}/kernel_main.o: ./kernel_main.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/krlglobal.o: ./kernel/krlglobal.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/krlinit.o: ./kernel/krlinit.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/krlmempool.o: kernel/krlmempool.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/virmem.o: ./kernel/virmem.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/thread.o: ./kernel/thread/thread.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/scheduler.o: ./kernel/thread/scheduler.c
	gcc ${CFLAGS_X64} ${INCLUDE} ${DEBUG} -c $< -o $@

${BUILD}/OSTool: ${ROOT_PATH}/tool/main.c
	gcc $< -o $@
	chmod 777 $<