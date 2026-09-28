PROJECT = fw_stm32
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

MCU = -mcpu=cortex-m3 -mthumb
DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD
INCLUDES = -I. -ICMSIS -ISPL/inc

CFLAGS = $(MCU) $(DEFS) $(INCLUDES) -O2 -Wall -fdata-sections -ffunction-sections
LDFLAGS = $(MCU) -specs=nano.specs -Tlinker.ld -Wl,-Map=$(PROJECT).map -Wl,--gc-sections

C_SOURCES = \
main.c \
CMSIS/system_stm32f10x.c \
SPL/src/stm32f10x_rcc.c \
SPL/src/stm32f10x_gpio.c \
SPL/src/stm32f10x_adc.c \
SPL/src/stm32f10x_dma.c \
SPL/src/stm32f10x_tim.c \
SPL/src/stm32f10x_usart.c \
SPL/src/misc.c

ASM_SOURCES = startup_stm32f10x_md.s

OBJS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)

all: $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(PROJECT).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SZ) $@

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary -S $< $@

clean:
	rm -f $(OBJS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin $(PROJECT).map