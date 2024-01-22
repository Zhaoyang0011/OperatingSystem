CFLAGS:= -m32 # 32 bit program
CFLAGS+= -masm=intel
CFLAGS+= -fno-builtin			# no gcc built-in function
CFLAGS+= -nostdinc				# no std head file
CFLAGS+= -fno-pic				# position independent code
CFLAGS+= -fno-pie				# position independent executable
CFLAGS+= -nostdlib				# no std lib
CFLAGS+= -fno-stack-protector	# no stack protector
CFLAGS:= $(strip ${CFLAGS})

CFLAGS_64:= -m64 # 64 bit program
#CFLAGS_64+= -masm=intel
CFLAGS_64+= -fno-builtin			# no gcc built-in function
CFLAGS_64+= -nostdinc				# no std head file
CFLAGS_64+= -fno-pic				# position independent code
CFLAGS_64+= -fno-pie				# position independent executable
CFLAGS_64+= -nostdlib				# no std lib
CFLAGS_64+= -fno-stack-protector	# no stack protector
CFLAGS_64+= -ffreestanding
CFLAGS_64:= $(strip ${CFLAGS_64})

DEBUG:= -g
