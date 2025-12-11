# 从零构建操作系统实验 (RISC-V OS)

本项目是在指导手册引导下，从零开始构建一个基于RISC-V架构的简单操作系统。


## 编译与运行说明

### 1. 环境要求
*   Ubuntu 22.04
*   `git`, `make`, `python3`
*   RISC-V交叉编译工具链 (`riscv64-unknown-elf-gcc`)
*   QEMU (`qemu-system-riscv64`)

### 2. 编译内核
在项目根目录下，执行以下命令进行编译：
make
该命令会编译所有内核源文件，并链接生成最终的可执行文件 kernel.elf。
编译成功后，执行以下命令来启动QEMU并运行内核：
make qemu
如果需要清理所有编译生成的文件（如.o, .elf文件），可以执行：
make clean
