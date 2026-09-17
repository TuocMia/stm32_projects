#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

uint16_t PrescalerValue = 0;

/* UART receive buffer and command state.
   rx_buffer stores the latest command typed from the serial monitor.
   cmd_ready tells the main loop a full command has been received. */
char rx_buffer[16];
uint8_t rx_index = 0;
volatile uint8_t cmd_ready = 0;

/* LED control status.
   led_state = 1 means LED is ON; 0 means OFF.
   led_brightness stores the brightness level in percent (0..100). */
uint8_t led_state = 0;
uint8_t led_brightness = 0; // Brightness level (0-100)

void RCC_Configuration(void);
void GPIO_Configuration(void);
void TIM2_Configuration(void);
void USART1_Configuration(void);
void UART_SendString(const char *str);

int main(void)
{
    /* Initialize all peripheral blocks used by the project:
       - clock tree
       - GPIO pins
       - timer for PWM output
       - UART1 for serial commands */
    RCC_Configuration();
    GPIO_Configuration();
    TIM2_Configuration();
    USART1_Configuration();

    /* Send startup message to the serial terminal. */
    UART_SendString("START\r\n");

    /* Main loop: wait for incoming UART command, parse it, and execute action. */
    while (1)
    {
        if(cmd_ready)
        {
            /* Command: turn LED on */
            if(strcmp(rx_buffer, "ON") == 0)
            {
                led_state = 1;
                TIM_SetCompare1(TIM2, led_brightness * 10); // Set duty cycle based on brightness
            }
            /* Command: turn LED off */
            else if(strcmp(rx_buffer, "OFF") == 0)
            {
                led_state = 0;
                TIM_SetCompare1(TIM2, 0);

            }
            /* Command: report current LED state and brightness */
            else if(strcmp(rx_buffer, "Status") == 0)
            {
                char status_msg[32];
                sprintf(status_msg, "LED is %s, PWM: %d%%\r\n", led_state ? "ON" : "OFF", led_brightness);
                UART_SendString(status_msg);
            }
            /* Command: set brightness, example: PWM:50% */
            else if(strncmp(rx_buffer, "PWM:", 4) == 0)
            {
                int new_brightness;
                if(sscanf(rx_buffer, "PWM:%d%%", &new_brightness) == 1)
                {
                    if(new_brightness >= 0 && new_brightness <= 100)
                    {
                        led_brightness = new_brightness;
                        if(led_state)
                        {
                            TIM_SetCompare1(TIM2, led_brightness * 10); // Update duty cycle
                        }
                    }
                }
            }

            /* Reset receive buffer so next command can be received. */
            rx_index = 0;
            memset(rx_buffer, 0, sizeof(rx_buffer));
            cmd_ready = 0;
        }
    }
}

void RCC_Configuration(void)
{
    /* Configure the clock for the peripherals used:
       - TIM2: timer used to generate PWM for the LED
       - GPIOA: used for PA0 (PWM), PA9 (TX), and PA10 (RX)
       - AFIO: enables alternate function capability
       - USART1: serial communication with the computer

       RCC_APB1PeriphClockCmd: enables the APB1 bus clock (timers, etc.)
       RCC_APB2PeriphClockCmd: enables the APB2 bus clock (GPIO, AFIO, USART1)
    */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef gpio;

    /* Configure GPIO for PWM and UART:
       - PA0: TIM2_CH1 -> PWM output to the LED
       - PA9: USART1_TX -> transmits serial data to the PC
       - PA10: USART1_RX -> receives serial data from the PC

       GPIO_Mode_AF_PP: alternate function, push-pull output mode
       GPIO_Mode_IN_FLOATING: RX pin receives floating input signals
    */
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);
}

void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef tim;
    TIM_OCInitTypeDef tim_oc;

    /* Configure Timer 2 to generate PWM for the LED.

       1) Set the timer base:
          - SystemCoreClock = 72 MHz
          - Prescaler = (72 MHz / 1 MHz) - 1 = 71
          -> timer clock after prescaler = 1 MHz

       2) Set the PWM period:
          - TIM_Period = 1000 - 1 = 999
          - PWM frequency = 1 MHz / 1000 = 1 kHz
          - one PWM cycle is 1 ms

       3) Brightness values from 0..100 are converted to duty cycle values from 0..1000:
          - 50% => 500
          - 100% => 1000
    */
    PrescalerValue = (uint16_t) (SystemCoreClock / 1000000) - 1; // 1 MHz timer clock

    tim.TIM_Period = 1000 - 1; // 1 kHz PWM frequency
    tim.TIM_Prescaler = PrescalerValue;
    tim.TIM_ClockDivision = 0;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim);

    tim_oc.TIM_OCMode = TIM_OCMode_PWM1;                // PWM mode 1
    tim_oc.TIM_OutputState = TIM_OutputState_Enable;    // Enable output channel
    tim_oc.TIM_Pulse = led_brightness * 10;             // Initial duty cycle value
    tim_oc.TIM_OCPolarity = TIM_OCPolarity_High;        // Active high polarity
    TIM_OC1Init(TIM2, &tim_oc);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);   // Enable PWM preload
    TIM_ARRPreloadConfig(TIM2, ENABLE);                 // Enable auto-reload register update
    TIM_Cmd(TIM2, ENABLE);                              // Start the timer
}

void USART1_Configuration(void)
{
    NVIC_InitTypeDef nvic;
    USART_InitTypeDef usart;

    /* Configure USART1 for serial communication with the PC:
       - Baudrate = 115200 bps
       - Word length = 8 bits
       - Stop bits = 1
       - Parity = none
       - Mode = Tx + Rx
       - No hardware flow control

       USART_ITConfig(..., USART_IT_RXNE, ENABLE): enable interrupt on new received data
       NVIC_Init: register the USART1_IRQn interrupt for ISR handling
    */
    usart.USART_BaudRate = 115200;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usart);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(USART1, ENABLE);
}

void UART_SendString(const char *str)
{
    /* Send a null-terminated string over USART1.
       This function is used to print messages like START and status replies. */
    while (*str)
    {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}

void USART1_IRQHandler(void)
{
    /* UART receive interrupt.
       - Read each incoming byte from the data register.
       - Store characters in rx_buffer until '!' arrives.
       - On '!', terminate the string and set cmd_ready = 1 so the main loop can parse it. */
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        char c = USART_ReceiveData(USART1);

        if(c == '!'){
            rx_buffer[rx_index] = '\0'; // Null-terminate the string
            cmd_ready = 1;
        }
        else if(rx_index < sizeof(rx_buffer) - 1 && !cmd_ready) {
            rx_buffer[rx_index++] = c;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}