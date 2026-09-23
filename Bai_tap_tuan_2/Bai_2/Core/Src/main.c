#include "main.h"

/*
 * Các biến đếm thời gian được thay đổi bên trong
 * hàm xử lý ngắt SysTick nên phải khai báo volatile.
 *
 * SysTick tạo ngắt mỗi 1 ms.
 *
 * LED1:
 * 0.1 Hz -> chu kỳ 10 giây
 * -> đảo trạng thái mỗi 5 giây
 * -> 5000 lần ngắt SysTick.
 *
 * LED2:
 * 1 Hz -> chu kỳ 1 giây
 * -> đảo trạng thái mỗi 500 ms
 * -> 500 lần ngắt SysTick.
 *
 * LED3:
 * 10 Hz -> chu kỳ 100 ms
 * -> đảo trạng thái mỗi 50 ms
 * -> 50 lần ngắt SysTick.
 */

volatile uint32_t led1_counter = 0;
volatile uint32_t led2_counter = 0;
volatile uint32_t led3_counter = 0;


/*
 * Khai báo các hàm khởi tạo ngoại vi.
 */

static void GPIO_Init(void);
static void SysTick_Init(void);


int main(void)
{
    /*
     * Khởi tạo HAL.
     *
     * HAL_Init() thực hiện các thiết lập cơ bản cho HAL,
     * đồng thời khởi tạo SysTick mặc định cho hệ thống.
     */
    HAL_Init();


    /*
     * Khởi tạo GPIO cho 3 LED.
     */
    GPIO_Init();


    /*
     * Cấu hình lại SysTick để tạo ngắt
     * với chu kỳ 1 ms.
     */
    SysTick_Init();


    /*
     * Vòng lặp chính không cần thực hiện công việc.
     *
     * Việc điều khiển 3 LED được thực hiện hoàn toàn
     * bên trong hàm xử lý ngắt SysTick.
     */
    while (1)
    {
    }
}


/* ============================================================
 *                         GPIO INIT
 * ============================================================ */

static void GPIO_Init(void)
{
    /*
     * Bật clock cho GPIOA.
     *
     * PA0, PA1 và PA2 đều thuộc GPIOA.
     */
    __HAL_RCC_GPIOA_CLK_ENABLE();


    GPIO_InitTypeDef GPIO_InitStruct = {0};


    /*
     * Cấu hình PA0, PA1 và PA2 làm ngõ ra
     * Push-Pull.
     *
     * GPIO_MODE_OUTPUT_PP:
     *     Output Push-Pull.
     *
     * GPIO_SPEED_FREQ_LOW:
     *     Tốc độ chuyển mức thấp là đủ cho việc
     *     điều khiển LED.
     */
    GPIO_InitStruct.Pin =
        LED1_PIN |
        LED2_PIN |
        LED3_PIN;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;


    /*
     * Áp dụng cấu hình cho GPIOA.
     */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /*
     * Đưa cả 3 LED về trạng thái ban đầu LOW.
     */
    HAL_GPIO_WritePin(
        GPIOA,
        LED1_PIN | LED2_PIN | LED3_PIN,
        GPIO_PIN_RESET
    );
}


/* ============================================================
 *                       SYSTICK INIT
 * ============================================================ */

static void SysTick_Init(void)
{
    /*
     * Cấu hình SysTick tạo ngắt mỗi 1 ms.
     *
     * HAL_RCC_GetHCLKFreq():
     *     Lấy tần số clock HCLK hiện tại.
     *
     * Chia cho 1000:
     *     HCLK / 1000 = số chu kỳ clock tương ứng
     *     với khoảng thời gian 1 ms.
     *
     * HAL_SYSTICK_Config() sẽ cấu hình thanh ghi
     * Reload của SysTick và bật ngắt SysTick.
     */
    HAL_SYSTICK_Config(
        HAL_RCC_GetHCLKFreq() / 1000
    );


    /*
     * Đặt mức ưu tiên cho ngắt SysTick.
     *
     * SysTick được đặt priority 0.
     * Trên STM32F1, số priority càng nhỏ thì
     * mức ưu tiên càng cao.
     */
    HAL_NVIC_SetPriority(
        SysTick_IRQn,
        0,
        0
    );
}