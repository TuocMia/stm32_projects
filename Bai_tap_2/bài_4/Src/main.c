#include "stm32f1xx_hal.h"                         // Thư viện HAL cho STM32F1

TIM_HandleTypeDef htim2;                           // Biến quản lý TIM2
void SystemClock_Config(void);                     // Hàm cấu hình clock hệ thống
void GPIOA_PWM_Init(void);                          // Hàm cấu hình chân GPIOA cho PWM
void TIM2_PWM_Init(void);                           // Hàm cấu hình TIM2 tạo PWM

int main(){
    HAL_Init();                                    // Khởi tạo thư viện HAL
    SystemClock_Config();                          // Cấu hình clock hệ thống lên 72 MHz
    GPIOA_PWM_Init();                              // Cấu hình PA0–PA3 làm chân PWM
    TIM2_PWM_Init();                               // Cấu hình TIM2 ở chế độ PWM

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);      // Bắt đầu PWM CH1 → PA0
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);      // Bắt đầu PWM CH2 → PA1
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);      // Bắt đầu PWM CH3 → PA2
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);      // Bắt đầu PWM CH4 → PA3

    while(1){
                                                    // TIM2 tự tạo PWM bằng phần cứng
    }
}

void SystemClock_Config(void){
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};    // Cấu hình nguồn dao động
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};    // Cấu hình clock hệ thống và bus

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; // Chọn HSE làm nguồn clock
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;       // Bật HSE
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;   // Bật PLL
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE; // Chọn HSE làm nguồn cho PLL
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;   // PLL nhân 9: 8 MHz × 9 = 72 MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);         // Áp dụng cấu hình HSE + PLL

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2; // Chọn các clock cần cấu hình
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // Chọn PLL làm SYSCLK

    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1; // HCLK = 72 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   // PCLK1 = 36 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;   // PCLK2 = 72 MHz

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2); // Áp dụng cấu hình clock
}

void GPIOA_PWM_Init(void){
    __HAL_RCC_GPIOA_CLK_ENABLE();                  // Bật clock cho GPIOA

    GPIO_InitTypeDef GPIO_InitStruct = {0};        // Tạo cấu hình GPIO

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |
                          GPIO_PIN_2 | GPIO_PIN_3; // Chọn PA0, PA1, PA2, PA3

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;        // Alternate Function Push-Pull
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;   // Tốc độ GPIO Low

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);        // Áp dụng cấu hình cho GPIOA
}

void TIM2_PWM_Init(void){

    __HAL_RCC_TIM2_CLK_ENABLE();                   // Bật clock cho TIM2

    htim2.Instance = TIM2;                         // Chọn bộ định thời TIM2
    htim2.Init.Prescaler = 71;                     // Chia clock: 72 MHz/(71+1) = 1 MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;   // Timer đếm tăng
    htim2.Init.Period = 999;                       // Đếm 0→999, tạo chu kỳ 1000 us = 1 ms
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // Không chia thêm clock
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Tắt preload ARR

    HAL_TIM_PWM_Init(&htim2);                      // Khởi tạo TIM2 ở chế độ PWM

    TIM_OC_InitTypeDef sConfigOC = {0};            // Cấu hình Output Compare/PWM
    sConfigOC.OCMode = TIM_OCMODE_PWM1;            // Chọn PWM Mode 1
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;    // PWM mức tích cực HIGH
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;     // Tắt Fast Mode

    sConfigOC.Pulse = 100;                         // CCR = 100 → Duty khoảng 10%
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1); // Cấu hình CH1

    sConfigOC.Pulse = 300;                         // CCR = 300 → Duty khoảng 30%
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2); // Cấu hình CH2

    sConfigOC.Pulse = 500;                         // CCR = 500 → Duty khoảng 50%
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3); // Cấu hình CH3

    sConfigOC.Pulse = 700;                         // CCR = 700 → Duty khoảng 70%
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4); // Cấu hình CH4
}
