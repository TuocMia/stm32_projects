
#include "stm32f10x.h"
#include "stm32f10x_conf.h" 
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
/* =========================================================
 * 1. KHAI BAO CAU HINH & BIEN TOAN CUC
 * ========================================================= */
#define ADC_BUFFER_SIZE 100U // 100 mau/giay tuong ung 100Hz

uint16_t adc_buffer[ADC_BUFFER_SIZE];

// Cac co bao hieu tu ngat DMA
volatile bool half_cplt_flag = false;
volatile bool full_cplt_flag = false;

// Nguyen mau ham
static void System_Init(void);
static void UART1_SendString(char *str);

/* =========================================================
 * 2. CHUONG TRINH CHINH (MAIN)
 * ========================================================= */
int main(void)
{
    char tx_buffer[20]; // Bo dem tam de chuyen so thanh chuoi

    // Khoi tao toan bo phan cung (Clock, GPIO, UART, DMA, ADC, Timer)
    System_Init();

    while (1)
    {
        /* --- KHI NUA DAU BUFFER DA DAY (Index 0 den 49) --- */
        // Luc nay CPU doc nua DAU, DMA dang ghi vao nua SAU -> Rat an toan
        if (half_cplt_flag)
        {
            half_cplt_flag = false; // Xoa co
            for (uint32_t i = 0; i < (ADC_BUFFER_SIZE / 2); i++)
            {
                // Chuyen doi so thanh chuoi, ket thuc bang \n\r
                sprintf(tx_buffer, "%u\n\r", adc_buffer[i]);
                UART1_SendString(tx_buffer);
            }
        }

        /* --- KHI NUA SAU BUFFER DA DAY (Index 50 den 99) --- */
        // Luc nay CPU doc nua SAU, DMA da vong lai ghi de nua DAU -> Rat an toan
        if (full_cplt_flag)
        {
            full_cplt_flag = false; // Xoa co
            for (uint32_t i = (ADC_BUFFER_SIZE / 2); i < ADC_BUFFER_SIZE; i++)
            {
                sprintf(tx_buffer, "%u\n\r", adc_buffer[i]);
                UART1_SendString(tx_buffer);
            }
        }
    }
}

/* =========================================================
 * 3. HAM TRUYEN DU LIEU UART
 * ========================================================= */
static void UART1_SendString(char *str)
{
    while (*str)
    {
        // Cho thanh ghi dich truyen trong (TXE)
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

/* =========================================================
 * 4. TRINH PHUC VU NGAT (INTERRUPT HANDLER)
 * ========================================================= */
void DMA1_Channel1_IRQHandler(void)
{
    // Kiem tra co ngat Half-Transfer (Da truyen xong nua mang)
    if (DMA_GetITStatus(DMA1_IT_HT1))
    {
        DMA_ClearITPendingBit(DMA1_IT_HT1);
        half_cplt_flag = true;
    }

    // Kiem tra co ngat Transfer-Complete (Da truyen xong ca mang)
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        full_cplt_flag = true;
    }
}

/* =========================================================
 * 5. KHOI TAO PHAN CUNG (SPL CONFIGURATION)
 * ========================================================= */
static void System_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // --- A. BAT XUNG NHIP (CLOCK) ---
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 | 
                           RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // Clock ADC = 72MHz / 6 = 12MHz

    // --- B. CAU HINH CHAN GPIO ---
    // PA1: ADC1 Channel 1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA9: UART1 TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // PA10: UART1 RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // --- C. CAU HINH UART1 (115200 Baudrate) ---
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx; // Chi can truyen TX
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    // --- D. CAU HINH DMA CHO ADC ---
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // Ngoai vi -> Bo nho
    DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // Tang dia chi RAM
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 16-bit
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // Che do vong lap tron (Ping-Pong)
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    // Bat ngat DMA va kich hoat
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // Bat bo dieu khien ngat NVIC cho DMA
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // --- E. CAU HINH ADC1 ---
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // Quan trong: Tat de cho Timer
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; // Trigger tu TIM3
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
    ADC_ExternalTrigConvCmd(ADC1, ENABLE); // Cho phep Trigger ngoai
    ADC_DMACmd(ADC1, ENABLE);              // Ket noi ADC voi DMA
    ADC_Cmd(ADC1, ENABLE);

    // Hieu chuan ADC (Bat buoc voi F103)
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // --- F. CAU HINH TIMER 3 (Tan so 100Hz) ---
    // Clock = 72MHz. Chia PSC = 7200 -> 10kHz. Chia ARR = 100 -> 100Hz.
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1; 
    TIM_TimeBaseStructure.TIM_Period = 100 - 1;     
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // Cau hinh phat su kien TRGO ra ngoai moi lan Timer Update (tran)
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    
    // Bat dau chay Timer (Luc nay ADC bat dau tu dong lay mau)
    TIM_Cmd(TIM3, ENABLE);
}