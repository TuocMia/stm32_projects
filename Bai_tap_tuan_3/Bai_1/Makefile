STD_LIB = ../../STM32F10x_StdPeriph_Lib_V3.5.0/Libraries

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g -Wall
CFLAGS += -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
CFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables
CFLAGS += -I.
CFLAGS += -I$(STD_LIB)/CMSIS/CM3/CoreSupport
CFLAGS += -I$(STD_LIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x
CFLAGS += -I$(STD_LIB)/STM32F10x_StdPeriph_Driver/inc

SOURCES = main.c
SOURCES += VL53L0X.c
SOURCES += $(STD_LIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
SOURCES += $(STD_LIB)/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
SOURCES += $(STD_LIB)/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
SOURCES += $(STD_LIB)/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c
SOURCES += $(STD_LIB)/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
SOURCES += $(STD_LIB)/STM32F10x_StdPeriph_Driver/src/misc.c

ASM_SOURCE = $(STD_LIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/gcc_ride7/startup_stm32f10x_md.s

LDFLAGS = -Tlinker.ld -nostartfiles --specs=nosys.specs

all: firmware.bin

firmware.elf:
	$(CC) $(CFLAGS) $(LDFLAGS) $(ASM_SOURCE) $(SOURCES) -o firmware.elf

firmware.bin: firmware.elf
	$(OBJCOPY) -O binary firmware.elf firmware.bin

clean:
	rm -f *.o *.elf *.bin

flash: firmware.bin
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program firmware.bin verify reset exit 0x08000000"
