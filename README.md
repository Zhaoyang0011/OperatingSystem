# Operating-System

[中文readme](./README_CHN.md)

In this project, I will implement a simple operating system and record my learning process.

## Build Project

Install qemu:
```
apt-get install qemu
```

Compile project:
```
make
```

Run the project through QEMU:
```
make qemu
```

Run the project through QEMU in debug mode:
```
make qemug
```

## Components

### Bootloader

A bootloader is a computer program that is responsible for booting a computer.

[Bootloader implementation](./note/bootloader.md)

### Kernel

[Kernel implementation](./note/kernel.md)