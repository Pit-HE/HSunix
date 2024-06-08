
CPUS = 1
QEMU = qemu-system-riscv64
TOOLPREFIX := riscv64-linux-gnu-



CC = $(TOOLPREFIX)gcc 
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump



s_src = entry.S trampoline.S kernelvec.S switch.S temp_retuser.S
c_src = $(wildcard *.c)
obj :=
obj +=$(patsubst %.S,%.o, ${s_src})
obj +=$(patsubst %.c,%.o, ${c_src})



CFLAGS := -Wall -Werror -O0 -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CFLAGS += -fno-pie -no-pie

LDFLAGS = -z max-page-size=4096

QEMUOPTS := -machine virt -bios none -kernel kernel -m 160M -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0


%.o:%.c
	$(CC)${CFLAGS} $^ -c -o $@
%.o:%.S
	$(CC)${CFLAGS} -c -o $@ $^



all:kernel
	make flush

flush:
	rm -rf *.d *.o *.asm *.out *.sym initcode

kernel: $(obj) kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(obj) 
	$(OBJDUMP) -S kernel > kernel.asm
	$(OBJDUMP) -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym

qemu:
	$(QEMU) $(QEMUOPTS)

debug:
	make all flush
	$(QEMU) $(QEMUOPTS) -S -gdb tcp::25000

clean:
	make flush
	rm -rf kernel

.PHONY: all clean debug qemu

