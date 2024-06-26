
# 编译工具的信息
QEMU = qemu-system-riscv64
TOOLPREFIX := riscv64-linux-gnu-
CC = $(TOOLPREFIX)gcc 
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

# 向下传递的宏
CPUS = 1
ROOT_DIR = $(shell pwd)


# 编译时的参数
CFLAGS := -Wall -Werror -O0 -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CFLAGS += -fno-pie -no-pie
CFLAGS += -I.

# 链接时的参数
LDFLAGS = -z max-page-size=4096

# 提供给子目录 makefile 的宏
export CC LD OBJCOPY OBJDUMP QEMU
export CFLAGS LDFLAGS 
export CPUS ROOT_DIR


# 默认清空工程重新编译并运行 HSunix
all: remove
	make build
	make clean
	make app
	make mkfs
	make fs.img
	make qemu

# 重新构建整个系统内核
build: remove
	make -C kernel

# 生成创建可挂载的磁盘文件 fs.img 的工具
mkfs:
	make -C mkfs

# 仅编译 user/app 中写的执行在用户空间的代码
app:
	make -C user

# 生成可以挂载到 qemu 上的磁盘文件
fs.img:
	cd user && make fs_img

# 运行 qemu 让操作系统开始模拟运行
qemu:
	cd kernel && make qemu

# 运行 qemu 让操作系统进入调试模式
debug:
	cd kernel && make debug

# 清除编译过程生成的中间文件
clean:
	cd kernel && make clean
	cd mkfs && make clean
	cd user && make clean

# 清除所有不属于项目源码的生成文件
remove: clean
	rm -rf kernel/HSunix kernel/HSunix.map



.PHONY: all clean debug qemu build remove fs.img app mkfs

