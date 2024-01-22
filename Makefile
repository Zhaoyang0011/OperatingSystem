include common.mk

ROOT_PATH:= $(shell pwd)
BUILD:=${ROOT_PATH}/build
TOOL:=${ROOT_PATH}/tool
BIN:=${ROOT_PATH}/bin
SRC:=${ROOT_PATH}/src
ARC?=x86_32
FILES:= ${BIN}/boot.bin
DIRS:= ${BUILD} ${BIN}
INCLUDES = -I./oskernel

HD_IMG_NAME:= "hd.img"

all: ${DIRS} ${FILES}
	dd if=${BIN}/boot.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc

${BIN}/boot.bin:
	$(MAKE) -C ${SRC}/boot/${ARC} ROOT_PATH=${ROOT_PATH}

${BUILD}:
	$(shell mkdir ${BUILD})

${BIN}:
	$(shell mkdir ${BIN})