#include "stm32f1xx_hal.h"

/*
 * Hàm xử lý ngắt SysTick.
 *
 * SysTick được HAL sử dụng làm bộ đếm thời gian hệ thống.
 * Mỗi lần xảy ra ngắt SysTick, HAL_IncTick() tăng bộ đếm thời gian
 * nội bộ của HAL.
 *
 * Bộ đếm này được sử dụng bởi các hàm xử lý thời gian,
 * điển hình là HAL_Delay().
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}