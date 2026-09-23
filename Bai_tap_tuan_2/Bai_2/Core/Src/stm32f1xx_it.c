#include "main.h"

/*
 * Các biến đếm thời gian được khai báo trong main.c.
 *
 * extern cho phép sử dụng các biến này
 * từ file stm32f1xx_it.c.
 */
extern volatile uint32_t led1_counter;
extern volatile uint32_t led2_counter;
extern volatile uint32_t led3_counter;


/*
 * ============================================================
 *                      SYSTICK HANDLER
 * ============================================================
 *
 * Hàm này được gọi tự động mỗi khi xảy ra
 * ngắt SysTick.
 *
 * SysTick được cấu hình với chu kỳ 1 ms,
 * vì vậy hàm này được thực thi mỗi 1 ms.
 */

void SysTick_Handler(void)
{
    /*
     * Tăng bộ đếm thời gian nội bộ của HAL.
     *
     * Hàm này rất quan trọng đối với HAL vì nó
     * duy trì bộ đếm thời gian của hệ thống.
     *
     * Các hàm như HAL_Delay() sử dụng bộ đếm này.
     */
    HAL_IncTick();


    /*
     * ========================================================
     *                         LED 1
     * ========================================================
     *
     * Tần số yêu cầu: 0.1 Hz
     *
     * Chu kỳ:
     *
     *     T = 1 / f
     *       = 1 / 0.1
     *       = 10 giây
     *
     * LED cần đảo trạng thái sau mỗi nửa chu kỳ:
     *
     *     10 / 2 = 5 giây
     *
     * Vì SysTick xảy ra mỗi 1 ms:
     *
     *     5 giây = 5000 ms
     *
     * Do đó LED1 được Toggle sau mỗi 5000 lần
     * ngắt SysTick.
     */

    led1_counter++;

    if (led1_counter >= 5000)
    {
        HAL_GPIO_TogglePin(
            LED1_PORT,
            LED1_PIN
        );

        led1_counter = 0;
    }


    /*
     * ========================================================
     *                         LED 2
     * ========================================================
     *
     * Tần số yêu cầu: 1 Hz
     *
     * Chu kỳ:
     *
     *     T = 1 / 1
     *       = 1 giây
     *
     * Đảo trạng thái sau:
     *
     *     1 / 2 = 0.5 giây
     *           = 500 ms
     *
     * Do đó LED2 được Toggle sau mỗi 500 lần
     * ngắt SysTick.
     */

    led2_counter++;

    if (led2_counter >= 500)
    {
        HAL_GPIO_TogglePin(
            LED2_PORT,
            LED2_PIN
        );

        led2_counter = 0;
    }


    /*
     * ========================================================
     *                         LED 3
     * ========================================================
     *
     * Tần số yêu cầu: 10 Hz
     *
     * Chu kỳ:
     *
     *     T = 1 / 10
     *       = 0.1 giây
     *       = 100 ms
     *
     * Đảo trạng thái sau:
     *
     *     100 / 2 = 50 ms
     *
     * Do đó LED3 được Toggle sau mỗi 50 lần
     * ngắt SysTick.
     */

    led3_counter++;

    if (led3_counter >= 50)
    {
        HAL_GPIO_TogglePin(
            LED3_PORT,
            LED3_PIN
        );

        led3_counter = 0;
    }
}