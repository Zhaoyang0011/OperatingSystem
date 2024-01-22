# Bootloader

I put the implementation of bootloader in the src/boot directory. For different hardware platforms we need to implement different bootloaders. So you can see that there are subdirectories under the boot directory, and each subdirectory corresponds to a hardware platform.

## x86_32

The code corresponding to this section is under directory `src/boot/x84_32`.

After starting under the x86 platform, the BIOS will first perform some initialization operations. This part of the operation is set by the manufacturer, and we do not need to know too much. After initialization, the BIOS will load the contents of the master boot sector into the memory address 0x7c00 and jump to 0x7c00. From there, our operating system takes over the entire platform.

Master boot record (MBR) is the boot sector of the hard disk. That is, it's what the BIOS loads and runs when booting the hard drive. The MBR is the first sector of the hard disk; it contains an MBR bootloader (described below).

`boot.asm` is our first code file and the starting point for our operating system. It is the bootloader we put in the MBR. It does three very simple operations: 

1. Initialize the register value. 
2. Use interrupt 13 to load the hard disk and load our secondary booter into memory. 
3. After loading is completed, jump to the secondary bootloader.