# 指定交叉编译工具链的前缀
TOOLPREFIX = riscv64-unknown-elf-

# 定义编译器、链接器等工具
CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld

# 定义编译和链接时使用的参数
# -mcmodel=medany: RISC-V特有的代码模型，适用于内核
# -ffreestanding: 告诉编译器我们是在一个“自由环境”（没有标准库）中
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
  kernel/mm/vm.o \
  kernel/trap/trap.o \
  kernel/trap/kernelvec.o \
  kernel/trap/trampoline.o \
  kernel/trap/sbi.o \
  kernel/proc/swtch.o \
  kernel/proc/proc.o \
  kernel/proc/ktest.o \
  kernel/proc/userinit.o \
  kernel/proc/syscall.o \
  kernel/proc/shell_bin.o \
  kernel/virtio_disk.o \
  kernel/fs/bio.o \
  kernel/fs/inode.o \
  kernel/fs/dir.o \
  kernel/fs/log.o \
  kernel/fs/fs.o

# 定义最终生成的可执行内核文件名
TARGET = kernel.elf

# 定义QEMU的命令和参数（让 QEMU 自动寻找默认的 OpenSBI）
QEMU = qemu-system-riscv64

QEMU_OPTS = -machine virt -nographic -kernel $(TARGET) \
            -drive file=fs.img,if=none,format=raw,id=x0 \
            -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

# 用户程序相关
USER_DIR = user
USER_SHELL = $(USER_DIR)/shell.bin
USER_SHELL_C = kernel/proc/shell_bin.c

# 文件系统镜像
FS_IMG = fs.img
MKFS = tools/mkfs

# --- 构建规则 ---

# 默认规则：输入make时执行
# 它依赖于$(TARGET)文件，所以会触发下面的链接规则
all: $(FS_IMG) $(TARGET)

# 编译 mkfs 工具（使用主机编译器）
$(MKFS): tools/mkfs.c
	@echo "Building mkfs tool..."
	@gcc -o $(MKFS) tools/mkfs.c

# 文件系统镜像生成
$(FS_IMG): $(MKFS)
	@echo "Creating empty file system image..."
	@./$(MKFS) $(FS_IMG)
	@echo "File system image created (6KB)"

# 用户程序编译规则
$(USER_SHELL):
	@echo "Building user programs..."
	@cd $(USER_DIR) && $(MAKE)

# 生成 shell_bin.c 的规则
$(USER_SHELL_C): $(USER_SHELL)
	@echo "Generating shell_bin.c..."
	@xxd -i $(USER_SHELL) > $(USER_DIR)/shell_bin_new.c
	@python3 scripts/gen_shell_bin.py $(USER_DIR)/shell_bin_new.c $(USER_SHELL_C)
	@rm -f $(USER_DIR)/shell_bin_new.c

# shell_bin.o 依赖于自动生成的 shell_bin.c
kernel/proc/shell_bin.o: $(USER_SHELL_C)

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

# 运行QEMU的规则：自动编译所有依赖
qemu: $(TARGET) $(FS_IMG)
	$(QEMU) $(QEMU_OPTS)

# 清理规则：删除所有生成的文件，只保留源代码
clean:
	@echo "Cleaning kernel build files..."
	@rm -f $(TARGET) $(OBJS)
	@echo "Cleaning user programs..."
	@cd $(USER_DIR) && $(MAKE) clean
	@echo "Cleaning generated files..."
	@rm -f $(USER_SHELL_C)
	@rm -f $(USER_DIR)/shell_bin.c
	@rm -f $(FS_IMG)
	@rm -f $(MKFS)
	@echo "Clean complete. Only source files (.c, .h, .S, .ld, Makefile, README) remain."
