# ============================================================
#                 CẤU HÌNH BUILD PROJECT
# ============================================================

# Tên file chương trình đầu ra
TARGET = firmware

# Bộ công cụ biên dịch cho kiến trúc ARM Cortex-M
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy


# ============================================================
#                 THƯ VIỆN STM32
# ============================================================

# Đường dẫn đến thư viện STM32CubeF1
CUBE = Drivers

CMSIS = $(CUBE)/CMSIS
HAL = $(CUBE)/STM32F1xx_HAL_Driver


# ============================================================
#                 TÙY CHỌN BIÊN DỊCH
# ============================================================

# STM32F103 sử dụng lõi ARM Cortex-M3
CFLAGS = -mcpu=cortex-m3 -mthumb

# -O2: tối ưu chương trình
# -Wall: bật cảnh báo của trình biên dịch
CFLAGS += -O2 -Wall

# Tách từng hàm/dữ liệu thành section riêng,
# cho phép linker loại bỏ phần không được sử dụng.
CFLAGS += -ffunction-sections -fdata-sections

# Xác định dòng vi điều khiển đang sử dụng
CFLAGS += -DSTM32F103xB

# Sử dụng thư viện STM32 HAL
CFLAGS += -DUSE_HAL_DRIVER


# ============================================================
#                 ĐƯỜNG DẪN HEADER
# ============================================================

CFLAGS += -ICore/Inc
CFLAGS += -I$(CMSIS)/Include
CFLAGS += -I$(CMSIS)/Device/ST/STM32F1xx/Include
CFLAGS += -I$(HAL)/Inc


# ============================================================
#                 TÙY CHỌN LINK
# ============================================================

# Sử dụng Cortex-M3 và linker script của project
LDFLAGS = -mcpu=cortex-m3 -mthumb
LDFLAGS += -Tstm32.ld

# Loại bỏ các section không được sử dụng
LDFLAGS += -Wl,--gc-sections

# Cung cấp các system call tối thiểu cho môi trường embedded
LDFLAGS += -specs=nosys.specs


# ============================================================
#                 DANH SÁCH SOURCE
# ============================================================

# Source code của ứng dụng và các thành phần HAL/CMSIS cần thiết
C_SOURCES = \
Core/Src/main.c \
Core/Src/stm32f1xx_it.c \
$(CMSIS)/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c \
$(HAL)/Src/stm32f1xx_hal.c \
$(HAL)/Src/stm32f1xx_hal_cortex.c \
$(HAL)/Src/stm32f1xx_hal_rcc.c \
$(HAL)/Src/stm32f1xx_hal_gpio.c \
$(HAL)/Src/stm32f1xx_hal_flash.c \
$(HAL)/Src/stm32f1xx_hal_flash_ex.c \
$(HAL)/Src/stm32f1xx_hal_adc.c \
$(HAL)/Src/stm32f1xx_hal_uart.c \
$(HAL)/Src/stm32f1xx_hal_rcc_ex.c \
$(HAL)/Src/stm32f1xx_hal_adc_ex.c


# Startup code của STM32F103
ASM_SOURCES = \
startup/startup_stm32f103xb.s


# Chuyển danh sách file source thành danh sách file object
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)


# ============================================================
#                     QUY TRÌNH BUILD
# ============================================================

# Mặc định tạo file firmware.bin
all: $(TARGET).bin


# Biên dịch file C thành file object
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Biên dịch file Assembly thành file object
%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@


# Link các file object thành file ELF
$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@


# Chuyển file ELF sang file BIN để nạp vào Flash của STM32
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@


# Xóa các file sinh ra trong quá trình build
clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).bin


# Nạp firmware vào STM32F103 thông qua ST-Link
flash: $(TARGET).bin
	st-flash --reset write $(TARGET).bin 0x08000000


# Khai báo các target không phải tên file
.PHONY: all clean flash
