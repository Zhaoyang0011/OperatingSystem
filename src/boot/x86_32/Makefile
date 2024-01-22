BUILD:=${ROOT_PATH}/build
BIN:=${ROOT_PATH}/bin

all: ${BIN}/boot.bin

${BIN}/boot.bin: ./boot.asm
	nasm $< -o $@