CFLAGS_X86:= -m32 # 32 bit program
CFLAGS_X86+= -masm=intel
CFLAGS_X86+= -fno-builtin			# no gcc built-in function
CFLAGS_X86+= -nostdinc				# no std head file
CFLAGS_X86+= -fno-pic				# position independent code
CFLAGS_X86+= -fno-pie				# position independent executable
CFLAGS_X86+= -nostdlib				# no std lib
CFLAGS_X86+= -fno-stack-protector	# no stack protector
CFLAGS_X86:= $(strip ${CFLAGS})

CFLAGS_X64:= -m64 # 64 bit program
#CFLAGS_64+= -masm=intel
CFLAGS_X64+= -fno-builtin			# no gcc built-in function
CFLAGS_X64+= -nostdinc				# no std head file
CFLAGS_X64+= -fno-pic				# position independent code
CFLAGS_X64+= -fno-pie				# position independent executable
CFLAGS_X64+= -nostdlib				# no std lib
CFLAGS_X64+= -fno-stack-protector	# no stack protector
CFLAGS_X64+= -ffreestanding
CFLAGS_X64:= $(strip ${CFLAGS_64})

DEBUG:= -g
