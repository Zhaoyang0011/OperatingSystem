# Bootloader



The bootloader is the first step of our operating system, that is, the first part of our operating system to be executed. In order to implement a bootloader, you need to have a certain understanding of the boot process of a specific platform such as x86, riscv or arm.

I put the implementation of bootloader in the src/boot directory. For different hardware platforms we need to implement different bootloaders. So you can see that there are subdirectories under the boot directory, and each subdirectory corresponds to a hardware platform.

## x86_64

The code corresponding to this section is under directory `src/boot/x84_32`.

### bootloader

After starting under the x86 platform, the BIOS will first perform some initialization operations. This part of the operation is designed by the manufacturer, and we do not need to know too much. After initialization, the BIOS will load the contents of the master boot sector into the memory address 0x7c00 and jump to 0x7c00. From there, our operating system takes over the entire platform.

Master boot record (MBR) is the boot sector of the hard disk. That is, it's what the BIOS loads and runs when booting the hard drive. The MBR is the first sector of the hard disk; it contains an MBR bootloader (described below).

`boot.asm` is our first code file and the starting point for our operating system. It is the bootloader we put in the MBR. It does three very simple operations: 

1. Initialize the register value. 
2. Use interrupt 13 to load the hard disk and load our secondary booter into memory. 
3. After loading is completed, jump to the secondary bootloader.

### Secondary loader

Since the bootloader only has 512Byte of space on the x86 platform. My kernel setting up code requires more memory. Therefore, I added a second-level bootloader to perform some setting up operations for the real kernel.

`setup.asm` is the entry point of the second-level bootloader. The last long jump operation of the bootloader is to jump to the code in this file for execution. It does the following things:

1. Call BIOS interrupt, explore the machine's memory information, and store it at memory address 0x5000.
2. Enter protected mode, so that we can access more than 1M of memory space.
3. Call the set_main function to initialize the kernel data and load the kernel into memory.
4. Jump to the kernel entry code after returning from set_main.
