# 指定交叉编译工具链的前缀
TOOLPREFIX = riscv64-unknown-elf-

# 定义编译器、链接器等工具
CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld

# 定义编译和链接时使用的参数
# -mcmodel=medany: RISC-V特有的代码模型，适用于内核
# -ffreestanding: 告诉编译器是在一个“自由环境”（没有标准库）中
# -nostdlib: 不链接标准库
# -g: 生成调试信息
# -Wall: 开启所有警告
# -Ikernel/: 允许 #include "..." 时在 kernel/ 目录中查找文件
CFLAGS = -mcmodel=medany -ffreestanding -nostdlib -g -Wall -Ikernel/ -Iinclude/

# 链接器参数，-T 指定使用我们的链接脚本
LDFLAGS = -T scripts/kernel.ld

# 定义所有需要被编译的源文件对应的.o目标文件
OBJS = \
  kernel/boot/entry.o \
  kernel/main.o \
  kernel/uart.o \
  kernel/printf.o \
  kernel/string.o \
  kernel/mm/pmm.o \
  kernel/mm/vm.o

# 定义最终生成的可执行内核文件名
TARGET = kernel.elf

# 定义QEMU的命令和参数
QEMU = qemu-system-riscv64
QEMU_OPTS = -machine virt -bios none -nographic -kernel $(TARGET)

# --- 构建规则 ---

# 默认规则：输入make时执行
# 它依赖于$(TARGET)文件，所以会触发下面的链接规则
all: $(TARGET)

# 链接规则：将所有的.o文件链接成最终的.elf文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# 通用编译规则：如何从.c文件生成.o文件
# $< 代表依赖文件 (源文件)，$@ 代表目标文件 (.o文件)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 通用编译规则：如何从.s文件生成.o文件
%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

# 运行QEMU的规则
qemu: $(TARGET)
	$(QEMU) $(QEMU_OPTS)

# 清理规则：删除所有生成的文件
clean:
	rm -f $(TARGET) $(OBJS)
