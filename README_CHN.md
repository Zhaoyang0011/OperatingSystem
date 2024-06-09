# 操作系统

在这个项目中，我会从零开始实现一个简单的操作系统并记录我的学习过程。

## 构建项目

安装 qemu：
```
apt-get install qemu
```

编译项目：
```
make
```

通过 QEMU 运行项目：
```
make qemu
```

以调试模式通过 QEMU 运行项目：
```
make qemug
```

## 组件

### 引导加载程序

引导加载程序是负责启动计算机的计算机程序。

[Bootloader实现](./note/chn/bootloader.md)