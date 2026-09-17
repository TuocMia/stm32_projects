#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H

/* Bật đầy đủ các module: GPIO, RCC, CORTEX, ADC, UART, DMA, FLASH */
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED

/* Định nghĩa tần số và thời gian Timeout */
#define HSE_VALUE              8000000U 
#define HSI_VALUE              8000000U 
#define HSE_STARTUP_TIMEOUT    100U
#define LSE_STARTUP_TIMEOUT    5000U
#define VDD_VALUE              3300U
#define TICK_INT_PRIORITY      0U
#define LSE_VALUE              32768U
#define LSI_VALUE              40000U

/* Bỏ qua kiểm tra lỗi để tiết kiệm bộ nhớ */
#define assert_param(expr)     ((void)0U)

/* Gọi các file header tương ứng */
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_flash.h"

#endif /* __STM32F1xx_HAL_CONF_H */