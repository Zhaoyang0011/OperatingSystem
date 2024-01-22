# Bootloader

I put the implementation of bootloader in the src/boot directory. For different hardware platforms we need to implement different bootloaders. So you can see that there are subdirectories under the boot directory, and each subdirectory corresponds to a hardware platform.

## x86_32

The code corresponding to this section is under directory `src/boot/x84_32`.

`boot.asm` is our first code file and the starting point for our operating system. It does three very simple operations: 

1. Initialize the register value. 
2. Use interrupt 13 to load the hard disk and load our secondary booter into memory. 
3. After loading is completed, jump to the secondary bootloader.