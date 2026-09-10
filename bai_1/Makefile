TOOLS = arm-none-eabi
CC = $(TOOLS)-gcc
OBJCOPY = $(TOOLS)-objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g -nostdlib -ffreestanding

all: firmware.bin

firmware.elf: startup_stm32f103.s main.c stm32f103.ld
	$(CC) $(CFLAGS) -T stm32f103.ld -o firmware.elf startup_stm32f103.s main.c

firmware.bin: firmware.elf
	$(OBJCOPY) -O binary firmware.elf firmware.bin

clean:
	rm -f *.o *.elf *.bin
