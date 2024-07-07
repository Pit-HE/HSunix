
include $(ROOT_DIR)/tools/tools.mak


# 获取当前 Makefile 所处的文件路径
CURRENT_DIR	:= $(shell pwd)

# 设置参与文件编译的头文件路径
INCLUDE_DIR += -I$(CURRENT_DIR)/arch
INCLUDE_DIR += -I$(CURRENT_DIR)/boot
INCLUDE_DIR += -I$(CURRENT_DIR)/cli
INCLUDE_DIR += -I$(CURRENT_DIR)/lib
INCLUDE_DIR += -I$(CURRENT_DIR)/src
INCLUDE_DIR += -I$(CURRENT_DIR)/fs
INCLUDE_DIR += -I$(CURRENT_DIR)/fs/disk_fs
INCLUDE_DIR += -I$(CURRENT_DIR)/fs/ram_fs
INCLUDE_DIR += -I$(CURRENT_DIR)/fs/virtual_fs

# 获取当前目录下的所有 C 文件
C_FILE += $(call all-c-files, arch)
C_FILE += $(call all-c-files, boot)
C_FILE += $(call all-c-files, cli)
C_FILE += $(call all-c-files, lib)
C_FILE += $(call all-c-files, src)
C_FILE += $(call all-c-files-in, ./fs)

# 获取当前目录下的所有汇编文件
S_FILE += $(call all-S-files, boot)
S_FILE += $(call all-S-files, arch)
S_FILE += $(call all-S-files, cli)
S_FILE += $(call all-S-files, lib)
S_FILE += $(call all-S-files, src)
S_FILE += $(call all-S-files-in, ./fs)

# 获取所有 C 文件对应的 o 文件列表
C_OBJ	+= $(patsubst %.c, %.o, $(C_FILE))
# 获取所有 S 文件对应的 o 文件列表
S_OBJ	+= $(patsubst %.S, %.o, $(S_FILE)) 


# 链接生成最终的 HSunix 系统内核的可执行文件
#
# all: $(C_OBJ) $(S_OBJ)
# 	$(LD) $(LDFLAGS) -Map HSunix.map -T kernel.ld -o HSunix $(S_OBJ) $(C_OBJ)
# 	$(OBJDUMP) -S HSunix > HSunix.asm
# 	$(OBJDUMP) -t HSunix | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > HSunix.sym
all: $(S_OBJ) $(C_OBJ)
	$(LD) $(LDFLAGS) -Map HSunix.map -T kernel.ld -o HSunix $(S_OBJ) $(C_OBJ)


# 用于 all 中，编译所有的 c 文件
$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $^
# 用于 all 中，编译所有的 S 文件
$(S_OBJ):%.o:%.S
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $^


# qemu 的参数
QEMUOPTS := -machine virt -bios none -kernel HSunix -m 128M -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=../fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

#  qemu 加载 HSunix 系统的可执行文件并进入调试模式
debug:
	$(QEMU) $(QEMUOPTS) -S -gdb tcp::25000

# qemu 加载 HSunix 系统内核的可执行文件
qemu:
	$(QEMU)  $(QEMUOPTS)

# 清除当前内核目录下生成的中间文件
clean:
	rm -rf ./arch/*.o ./arch/*.d
	rm -rf ./boot/*.o ./boot/*.d
	rm -rf ./cli/*.o ./cli/*.d
	rm -rf ./lib/*.o ./lib/*.d
	rm -rf ./src/*.o ./src/*.d
	rm -rf ./fs/*.o ./fs/*.d
	rm -rf ./fs/ram_fs/*.o ./fs/ram_fs/*.d
	rm -rf ./fs/disk_fs/*.o ./fs/disk_fs/*.d
	rm -rf ./fs/virtual_fs/*.o ./fs/virtual_fs/*.d
	rm -rf HSunix.asm HSunix.sym

remove:
	rm -rf HSunix HSunix.map
