
include $(ROOT_DIR)/tools/tools.mak

# 清空变量的内容
INCLUDE_DIR := 
C_FILE := 
C_OBJ := 

# 获取当前 Makefile 所处的文件路径
CURRENT_DIR := $(shell pwd)

# 设置参与文件编译的头文件路径
INCLUDE_DIR += -I$(CURRENT_DIR)/app
INCLUDE_DIR += -I$(CURRENT_DIR)/lib

# 获取当前目录下的所有 C 文件
C_FILE	+= $(call all-c-files, app)
C_FILE	+= $(call all-c-files, lib)
# 获取当前目录下的所有汇编文件
S_FILE	+= $(call all-S-files, lib)
# 获取所有 C 文件对应的 o 文件列表
C_OBJ	+= $(patsubst %.c, %.o, $(C_FILE)) 
# 获取所有汇编文件对应的 o 文件列表
S_OBJ	+= $(patsubst %.S, %.o, $(S_FILE)) 
# 获取指定路径下的所有 o 文件列表
LIB_OBJ += $(call all-o-files, lib)

# 获取 app 中所有 c 文件的名字 (没有后缀)
BIN_OBJ += $(basename $(notdir $(wildcard app/*.c)))


# 列举所有需要写入到磁盘文件 fs.img 中的目标
U = bin
UPROGS=\
	$U/_cat\
	$U/_echo\
	$U/_forktest\
	$U/_grep\
	$U/_init\
	$U/_kill\
	$U/_ln\
	$U/_ls\
	$U/_mkdir\
	$U/_rm\
	$U/_sh\
	$U/_stressfs\
	$U/_usertests\
	$U/_grind\
	$U/_wc\
	$U/_zombie\
	$U/_test\

# 编译所有用户层的源码文件并链接生成可执行文件
all: $(C_OBJ) $(S_OBJ)
	make build_ld

# 执行链接功能，生成最终的可执行文件对象
build_ld: $(BIN_OBJ)

# 创建用于挂载到 qemu 的磁盘文件：fs.img
fs_img:
	../mkfs/mkfs ../fs.img $(UPROGS)



# 编译所有的 .c 文件
$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $^
# 编译所有的 .S 文件
$(S_OBJ):%.o:%.S
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $^
# 在 bin 目录生成可执行文件
$(BIN_OBJ):%: app/%.o
	$(LD) $(LDFLAGS) -T user.ld -o bin/_$@ $^ $(LIB_OBJ)


clean:
	rm -rf app/*.o app/*.d
	rm -rf lib/*.o lib/*.d
