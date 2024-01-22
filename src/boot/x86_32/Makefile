BUILD:=${ROOT_PATH}/build
BIN:=${ROOT_PATH}/bin

all: ${BIN}/boot.bin ${BIN}/setup.bin

${BIN}/boot.bin: ./boot.asm
	nasm $< -o $@

${BIN}/setup.bin: ./setup.asm
	nasm $< -o $@