# Bootloader

我把对bootloader的实现放在`src/boot`目录下。对于不同的硬件平台我们需要实现不同的bootloader。因此你可以看到boot目录下有多个子目录，每个子目录对应一个硬件平台。

## x86_32

```
ORG 0x7c00

[BITs 16]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

```