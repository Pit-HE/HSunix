
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


all: qemu

build:
	make -C kernel

mkfs:
	make -C mkfs

app:
	make -C user

# mkfs/mkfs: mkfs/mkfs.c
# 	gcc -Werror -Wall -o mkfs/mkfs mkfs/mkfs.c
mkfs/mkfs: mkfs/mkfs.c
	gcc mkfs/mkfs.c -o mkfs/mkfs

fs.img:
	cd user && make fs_img

qemu:
	cd kernel && make qemu

debug:
	cd kernel && make debug

clean:
	cd kernel && make clean
	cd mkfs && make clean
	cd user && make clean

remove: clean
	rm -rf kernel/HSunix kernel/HSunix.map



.PHONY: all clean debug qemu build remove fs.img app mkfs

