
INCLUDE_DIR := 
C_FILE := 
C_OBJ := 

CURRENT_DIR := $(shell pwd)

INCLUDE_DIR += -I$(CURRENT_DIR)/app
INCLUDE_DIR += -I$(CURRENT_DIR)/lib

C_FILE	+= $(wildcard $(CURRENT_DIR)/app/*.c)
C_FILE	+= $(wildcard $(CURRENT_DIR)/lib/*.c)
S_FILE	+= $(wildcard $(CURRENT_DIR)/lib/*.S)
C_OBJ	+= $(patsubst %.c, %.o, $(C_FILE)) 
S_OBJ	+= $(patsubst %.S, %.o, $(S_FILE)) 

BIN_OBJ += $(basename $(notdir $(wildcard app/*.c)))
LIB_OBJ += $(wildcard $(CURRENT_DIR)/lib/*.o)

U = ubin
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
	$(LD) $(LDFLAGS) -T user.ld -o ubin/_$@ $^ $(LIB_OBJ)


clean:
	rm -rf app/*.o app/*.d
	rm -rf lib/*.o lib/*.d
