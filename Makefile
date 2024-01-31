include common.mk

ROOT_PATH:= $(shell pwd)
BUILD:=${ROOT_PATH}/build
TOOL:=${ROOT_PATH}/tool
BIN:=${ROOT_PATH}/bin
SRC:=${ROOT_PATH}/src
ARC?=x86_64
FILES:= ${BIN}/boot.bin ${BIN}/setup.bin ${BIN}/kernel.pkg
DIRS:= ${BUILD} ${BIN}

DEBUG:= -g

HD_IMG_NAME:= "hd.img"

all: clean ${DIRS} ${FILES}
	dd if=${BIN}/boot.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc
	dd if=${BIN}/setup.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=1 count=4 conv=notrunc
	dd if=${BIN}/kernel.pkg of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=5 count=200 conv=notrunc

${BIN}/boot.bin ${BIN}/setup.bin:
	$(MAKE) -C ${SRC}/boot/${ARC} ROOT_PATH=${ROOT_PATH}

${BIN}/kernel.pkg:
	$(MAKE) -C ${SRC}/kernel/${ARC} ROOT_PATH=${ROOT_PATH}

${BUILD}:
	$(shell mkdir ${BUILD})

${BIN}:
	$(shell mkdir ${BIN})

clean:
	rm -rf ${BUILD}
	rm -rf ${BIN}

QEMU:=: qemu-system-i386

ifeq ($(ARC), x86_64)
	QEMU:= qemu-system-x86_64
else ifeq ($(ARC), x86_32)
	QEMU:= qemu-system-i386
endif

qemu:
	qemu-system-x86_64 -m 2048 -hda $(BUILD)/$(HD_IMG_NAME)

qemug:
	qemu-system-x86_64 -m 2048 -hda $(BUILD)/$(HD_IMG_NAME) -S -s