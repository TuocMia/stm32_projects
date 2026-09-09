CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -nostdlib -T stm32.ld

all: blink.bin

blink.elf: main.c stm32.ld
	$(CC) $(CFLAGS) main.c -o blink.elf

blink.bin: blink.elf
	$(OBJCOPY) -O binary blink.elf blink.bin

clean:
	rm -f *.elf *.bin