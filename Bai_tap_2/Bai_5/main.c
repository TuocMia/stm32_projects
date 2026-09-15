#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include <string.h>
#include <stdlib.h>

#define RX_BUFFER_SIZE 64

/* Biến nhận UART qua ngắt */
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;

/* Trạng thái hệ thống */
uint8_t led_state = 0;      // 0: OFF, 1: ON
uint16_t last_percent = 50; // Mặc định 50% khi bật lần đầu

/* Nguyên mẫu hàm */
void RCC_Configuration(void);
void GPIO_Configuration(void);
void TIM2_PWM_Init(void);
void USART1_Init_Interrupt(void);
void UART1_SendChar(char ch);
void UART1_SendString(const char *str);
void UART1_SendNumber(uint32_t num);
void Process_Command(char *cmd);

int main(void)
{
    RCC_Configuration();
    GPIO_Configuration();
    TIM2_PWM_Init();
    USART1_Init_Interrupt();

    /* Khởi tạo ban đầu: Đèn OFF, duty = 0 */
    TIM_SetCompare1(TIM2, 0);

    UART1_SendString("\r\n=== STM32F103 PWM LED Command Interface Ready ===\r\n");
    UART1_SendString("Cac lenh hop le: ON!, OFF!, PWM:Percent%!, Status!\r\n");

    while (1)
    {
        if (command_ready)
        {
            Process_Command((char *)rx_buffer);
            command_ready = 0; // Xóa cờ để nhận lệnh tiếp theo
        }
    }
}

/* ================== Cấu hình Clock ================== */
void RCC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
}

/* ================== Cấu hình GPIO ================== */
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef gpio;

    /* PA0: TIM2_CH1 (PWM Output) */
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PA9: USART1_TX */
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PA10: USART1_RX */
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);
}

/* ================== Cấu hình PWM TIM2 ================== */
void TIM2_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef tim;
    TIM_OCInitTypeDef tim_oc;

    /* Clock counter = 72MHz / 72 = 1MHz */
    uint16_t prescalerValue = (uint16_t)(SystemCoreClock / 1000000) - 1;

    /* Tần số PWM = 1MHz / 1000 = 1kHz (độ phân giải 0 - 1000) */
    tim.TIM_Period = 1000 - 1;
    tim.TIM_Prescaler = prescalerValue;
    tim.TIM_ClockDivision = 0;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim);

    tim_oc.TIM_OCMode = TIM_OCMode_PWM1;
    tim_oc.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc.TIM_Pulse = 0;
    tim_oc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &tim_oc);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

/* ================== Cấu hình USART1 & Ngắt NVIC ================== */
void USART1_Init_Interrupt(void)
{
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    usart.USART_BaudRate = 115200;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart);

    /* Bật ngắt nhận USART1 (RXNE) */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    /* Cấu hình NVIC cho kênh ngắt USART1 */
    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(USART1, ENABLE);
}

/* ================== Trình phục vụ ngắt USART1 ================== */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        char ch = (char)USART_ReceiveData(USART1);

        /* Nếu chưa xử lý xong lệnh trước, bỏ qua byte mới để tránh tràn */
        if (!command_ready)
        {
            if (ch == '!')
            {
                rx_buffer[rx_index] = '\0'; // Kết thúc chuỗi C
                rx_index = 0;
                command_ready = 1;          // Báo main xử lý
            }
            else if (ch != '\r' && ch != '\n')
            {
                if (rx_index < (RX_BUFFER_SIZE - 1))
                {
                    rx_buffer[rx_index++] = ch;
                }
            }
        }
    }
}

/* ================== Xử lý chuỗi lệnh ================== */
void Process_Command(char *cmd)
{
    /* Lệnh ON */
    if (strcmp(cmd, "ON") == 0)
    {
        led_state = 1;
        /* Chuyển sang ON: đặt duty tương ứng với last_percent (0-100% -> 0-1000) */
        TIM_SetCompare1(TIM2, last_percent * 10);
        UART1_SendString("[ACK] LED da BAT, do sang: ");
        UART1_SendNumber(last_percent);
        UART1_SendString("%\r\n");
    }
    /* Lệnh OFF */
    else if (strcmp(cmd, "OFF") == 0)
    {
        led_state = 0;
        TIM_SetCompare1(TIM2, 0);
        UART1_SendString("[ACK] LED da TAT\r\n");
    }
    /* Lệnh PWM:Percent% (Ví dụ: PWM:75%) */
    else if (strncmp(cmd, "PWM:", 4) == 0)
    {
        char *ptr = cmd + 4;
        int percent = atoi(ptr);

        if (percent >= 0 && percent <= 100)
        {
            last_percent = (uint16_t)percent;

            if (led_state == 1)
            {
                /* Đang ON: cập nhật độ sáng thực tế ngay lập tức */
                TIM_SetCompare1(TIM2, last_percent * 10);
                UART1_SendString("[ACK] Cap nhat do sang truc tiep: ");
            }
            else
            {
                /* Đang OFF: chỉ lưu cấu hình, không thay đổi độ sáng thực tế */
                UART1_SendString("[ACK] Luu cau hinh (Den dang OFF): ");
            }
            UART1_SendNumber(last_percent);
            UART1_SendString("%\r\n");
        }
        else
        {
            UART1_SendString("[ERR] Gia tri Percent khong hop le (0-100)!\r\n");
        }
    }
    /* Lệnh Status */
    else if (strcmp(cmd, "Status") == 0)
    {
        UART1_SendString("[STATUS] Trang thai: ");
        if (led_state == 1)
        {
            UART1_SendString("ON | Do sang: ");
        }
        else
        {
            UART1_SendString("OFF | Muc cau hinh gan nhat: ");
        }
        UART1_SendNumber(last_percent);
        UART1_SendString("%\r\n");
    }
    else
    {
        UART1_SendString("[ERR] Lenh khong xac dinh: ");
        UART1_SendString(cmd);
        UART1_SendString("\r\n");
    }
}

/* ================== Các hàm gửi UART ================== */
void UART1_SendChar(char ch)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}

void UART1_SendNumber(uint32_t num)
{
    char buf[12];
    int i = 0;

    if (num == 0)
    {
        UART1_SendChar('0');
        return;
    }

    while (num > 0)
    {
        buf[i++] = (char)((num % 10) + '0');
        num /= 10;
    }

    while (i > 0)
    {
        UART1_SendChar(buf[--i]);
    }
}