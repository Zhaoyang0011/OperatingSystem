include common.mk

BUILD:=./build
TOOL:=./tool
BIN:=./bin
ARC?=x86_32
FILES = ${BIN}/boot.bin
INCLUDES = -I./oskernel

HD_IMG_NAME:= "hd.img"