

INCLUDE_DIR += -I../kernel/arch
INCLUDE_DIR += -I../kernel/boot
INCLUDE_DIR += -I../kernel/cli
INCLUDE_DIR += -I../kernel/lib
INCLUDE_DIR += -I../kernel/src
INCLUDE_DIR += -I../kernel/fs

all:
	gcc -Werror -Wall $(INCLUDE_DIR) -o makefs mkfs.c

clean:
	rm -rf makefs
