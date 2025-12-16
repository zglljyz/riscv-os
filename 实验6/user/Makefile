TOOLPREFIX = riscv64-unknown-elf-
CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy

CFLAGS = -march=rv64g -mabi=lp64 -ffreestanding -nostdlib -static -fno-common -g -Wall -I.
LDFLAGS = -T user.ld

PROGS = shell

all: $(PROGS)

shell: entry.o usys.o ulib.o shell.o
	$(LD) $(LDFLAGS) -o shell.elf $^
	$(OBJCOPY) -O binary shell.elf shell.bin

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.elf *.bin $(PROGS) _nice _sh

.PHONY: all clean
