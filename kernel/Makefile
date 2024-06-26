

CURRENT_DIR	:= $(shell pwd)


INCLUDE_DIR += -I$(CURRENT_DIR)/arch
INCLUDE_DIR += -I$(CURRENT_DIR)/boot
INCLUDE_DIR += -I$(CURRENT_DIR)/cli
INCLUDE_DIR += -I$(CURRENT_DIR)/lib
INCLUDE_DIR += -I$(CURRENT_DIR)/src
INCLUDE_DIR += -I$(CURRENT_DIR)/fs

C_FILE	+= $(wildcard $(CURRENT_DIR)/arch/*.c)
C_FILE	+= $(wildcard $(CURRENT_DIR)/boot/*.c)
C_FILE	+= $(wildcard $(CURRENT_DIR)/cli/*.c)
C_FILE	+= $(wildcard $(CURRENT_DIR)/lib/*.c)
C_FILE	+= $(wildcard $(CURRENT_DIR)/src/*.c)
C_FILE	+= $(wildcard $(CURRENT_DIR)/fs/*.c)

S_FILE	+= $(wildcard $(CURRENT_DIR)/boot/*.S)
S_FILE	+= $(wildcard $(CURRENT_DIR)/arch/*.S)
S_FILE	+= $(wildcard $(CURRENT_DIR)/cli/*.S)
S_FILE	+= $(wildcard $(CURRENT_DIR)/lib/*.S)
S_FILE	+= $(wildcard $(CURRENT_DIR)/src/*.S)
S_FILE	+= $(wildcard $(CURRENT_DIR)/fs/*.S)

C_OBJ	+= $(patsubst %.c, %.o, $(C_FILE)) 
S_OBJ	+= $(patsubst %.S, %.o, $(S_FILE)) 


# all: $(C_OBJ) $(S_OBJ)
# 	$(LD) $(LDFLAGS) -Map HSunix.map -T kernel.ld -o HSunix $(S_OBJ) $(C_OBJ)
# 	$(OBJDUMP) -S HSunix > HSunix.asm
# 	$(OBJDUMP) -t HSunix | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > HSunix.sym

all: $(S_OBJ) $(C_OBJ)
	$(LD) $(LDFLAGS) -Map HSunix.map -T kernel.ld -o HSunix $(S_OBJ) $(C_OBJ)


$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $^
$(S_OBJ):%.o:%.S
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $^


# qemu 的参数
# QEMUOPTS := -machine virt -bios none -kernel HSunix -m 128M -smp $(CPUS) -nographic
QEMUOPTS := -machine virt -bios none -kernel HSunix -m 128M -smp 1 -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=../fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

# debug:
# 	$(QEMU) $(QEMUOPTS) -S -gdb tcp::25000

debug:
	qemu-system-riscv64 $(QEMUOPTS) -S -gdb tcp::25000

qemu:
	qemu-system-riscv64 $(QEMUOPTS)

clean:
	rm -rf ./arch/*.o ./arch/*.d
	rm -rf ./boot/*.o ./boot/*.d
	rm -rf ./cli/*.o ./cli/*.d
	rm -rf ./lib/*.o ./lib/*.d
	rm -rf ./src/*.o ./src/*.d
	rm -rf ./fs/*.o ./fs/*.d
	rm -rf HSunix.asm HSunix.sym










