#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define NUM_LED      3
#define PWM_PERIOD   1000        
#define DUTY_STEP    100          

static QueueHandle_t led_queue[NUM_LED];
static volatile uint16_t *led_ccr[NUM_LED] = { &TIM2->CCR1, &TIM2->CCR2, &TIM2->CCR3 };

void HW_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef oc;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* PA0, PA1, PA2: TIM2 CH1..CH3 (PWM) */
    gpio.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PB0, PB1, PB10: ipu button */
    gpio.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &gpio);

    /* f_output = f_sys / (PSC + 1) * (ARR + 1) */
    tb.TIM_Prescaler     = (SystemCoreClock / 1000000) - 1;
    tb.TIM_Period        = PWM_PERIOD - 1;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tb);

    oc.TIM_OCMode       = TIM_OCMode_PWM1;
    oc.TIM_OutputState  = TIM_OutputState_Enable;
    oc.TIM_OCPolarity   = TIM_OCPolarity_High;
    oc.TIM_Pulse        = 0;
    TIM_OC1Init(TIM2, &oc);
    TIM_OC2Init(TIM2, &oc);
    TIM_OC3Init(TIM2, &oc);

    TIM_Cmd(TIM2, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin   = GPIO_Pin_13;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

/* LED task: wait for a new brightness value from the queue and update PWM */
void led_task(void *param)
{
    uint32_t id = (uint32_t)param;
    uint16_t duty;

    for (;;) {
        if (xQueueReceive(led_queue[id], &duty, portMAX_DELAY) == pdPASS) {
            *led_ccr[id] = duty;
        }
    }
}

/* Button task: scan the three buttons and send brightness to the selected LED */
void button_task(void *param)
{
    uint8_t  selected = 0;
    uint16_t duty[NUM_LED] = { 100, 100, 100 };   /* start at 10% brightness */
    uint8_t  prev_sel = 1, prev_up = 1, prev_down = 1;
    uint8_t  sel, up, down;
    int i;

    for (i = 0; i < NUM_LED; i++) {                 /* initial brightness */
        xQueueSend(led_queue[i], &duty[i], 0);
    }

    for (;;) {
        sel  = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
        up   = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
        down = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);
        if ((prev_sel && !sel) || (prev_up && !up) || (prev_down && !down)) {
    GPIOC->ODR ^= GPIO_Pin_13;      /* toggle PC13 on each button press */
}
        if (prev_sel && !sel) {                     /* button 1: select the next LED */
            selected = (selected + 1) % NUM_LED;
        }
        if (prev_up && !up && duty[selected] < PWM_PERIOD) {      /* button 2: increase brightness */
            duty[selected] += DUTY_STEP;
            xQueueSend(led_queue[selected], &duty[selected], 0);
        }
        if (prev_down && !down && duty[selected] > 0) {           /* button 3: decrease brightness */
            duty[selected] -= DUTY_STEP;
            xQueueSend(led_queue[selected], &duty[selected], 0);
        }

        prev_sel = sel; prev_up = up; prev_down = down;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    HW_Init();

    for (uint32_t i = 0; i < NUM_LED; i++) {
        led_queue[i] = xQueueCreate(4, sizeof(uint16_t));
    }

    xTaskCreate(led_task, "LED1", configMINIMAL_STACK_SIZE, (void *)0, 1, NULL);
    xTaskCreate(led_task, "LED2", configMINIMAL_STACK_SIZE, (void *)1, 1, NULL);
    xTaskCreate(led_task, "LED3", configMINIMAL_STACK_SIZE, (void *)2, 1, NULL);
    xTaskCreate(button_task, "BTN", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1) {}
}