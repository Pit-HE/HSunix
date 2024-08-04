
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
CFLAGS += -MD -Wno-int-to-pointer-cast
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
all: remove build app

# 重新构建内核与用户代码
build:
	make -C kernel
	make app
	cd user && make fs_img

# 生成创建可挂载的磁盘文件 fs.img 的工具
mkfs:
	make -C mkfs

# 生成可以挂载到 qemu 上的磁盘文件
fs.img:mkfs
	cd user && make fs_img

# 仅重新构建系统内核
kernel:
	make -C kernel

# 仅编译 user/app 中写的执行在用户空间的代码
app:mkfs
	rm -rf user/bin && mkdir user/bin
	cp -rf kernel/lib/syscall/syscall.h user/lib/
	cp -rf kernel/fs/fcntl.h user/lib/
	make -C user
	cd user && make fs_img

# 运行 qemu 让操作系统开始模拟运行
qemu:build
	cd kernel && make qemu

# 运行 qemu 让操作系统进入调试模式
debug:build
	cd kernel && make debug

# 清除编译过程生成的中间文件
clean:
	cd kernel && make clean
	cd mkfs && make clean
	cd user && make clean

# 清除所有不属于项目源码的生成文件
remove: clean
	rm -rf fs.img
	cd kernel && make remove
	cd mkfs && make remove
	cd user && make remove

# 自动安装 HSunix 系统的编译、运行的环境
install:
	sudo apt install libc6-riscv64-cross
	sudo apt install binutils-riscv64-linux-gnu
	sudo apt install gcc-riscv64-linux-gnu
	sudo apt install binutils-riscv64-unknown-elf
	sudo apt install gcc-riscv64-unknown-elf
	sudo apt install qemu
	sudo apt install qemu-system-riscv64

# 统计当前项目有效代码的总行数
statis:
	find . "(" -name "*.c" -or -name "*.h" -or -name "*.S" ")" -print | xargs wc -l


.PHONY: all clean debug qemu build remove fs.img app mkfs

