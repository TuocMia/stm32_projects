#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

/* Mang toan cuc chua ban tin de DMA doc */
char tx_buffer[100]; 

void SystemClock_Config(void);
void UART1_DMA_Config(void);
void Button_Config(void);
void Delay_ms(uint32_t ms);

int main(void) {
    uint32_t btn_count = 0;
    
    /* 1. Cau hinh he thong */
    SystemClock_Config();
    UART1_DMA_Config();
    Button_Config();
    
    while (1) {
        /* 2. Quet nut nhan PA0 (Tich cuc muc thap do dung Pull-up) */
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
            
            /* Chong doi phim (Debounce) co hoc bang phan mem */
            Delay_ms(20); 
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
                
                /* Tang bien dem */
                btn_count++;
                
                /* 3. Tao ban tin chuan theo yeu cau: <ID-Lop><ID Nhom>:BTN:<Gia tri> */
                sprintf(tx_buffer, "ELE1415_Nhom_02:BTN:%lu\r\n", btn_count);
                
                /* 4. Kich hoat DMA truyen du lieu (Khong su dung ham cho UART) */
                DMA_Cmd(DMA1_Channel4, DISABLE); /* Tat kenh DMA de thiet lap lai */
                DMA_SetCurrDataCounter(DMA1_Channel4, strlen(tx_buffer)); /* Nap do dai chuoi */
                DMA_Cmd(DMA1_Channel4, ENABLE); /* Kich hoat DMA tu dong chuyen du lieu ra UART */
                
                /* Cho nguoi dung nha phim de tranh dem lien tuc */
                while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET);
                Delay_ms(20); /* Debounce luc nha phim */
            }
        }
    }
}

/* --- PHAN CAU HINH (LAY TU EXAMPLE CUA ST) --- */

void Button_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /* Cau hinh PA0 lam Input, keo tro noi len 3.3V (Pull-up) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void UART1_DMA_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    /* Bat Clock cho GPIOA, USART1 va DMA1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* Cau hinh TX (PA9) la Alternate Function Push-Pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Cau hinh RX (PA10) la Input Floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Cau hinh USART1: 9600-8-N-1 */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    /* Cau hinh DMA1 Channel 4 phuc vu USART1_TX */
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tx_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; /* Chuyen du lieu tu RAM ra Ngoai vi */
    DMA_InitStructure.DMA_BufferSize = 0; /* Khoi tao bang 0, se duoc cap nhat khi bam nut */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; /* Truyen 1 lan roi dung */
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    /* Lien ket USART1 voi DMA */
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

void SystemClock_Config(void) {
    /* Ham chuan cua CMSIS de thiet lap System Clock */
    SystemInit();
}

/* Ham delay bang vong lap lap trinh */
void Delay_ms(uint32_t ms) {
    /* Uoc luong thoi gian delay cho xung nhip 72MHz. 
       Su dung tam thoi cho muc dich debounce phim bam. */
    for (uint32_t i = 0; i < ms * 8000; i++) {
        __NOP(); /* Lenh Assembly: No Operation - tieu ton cycle CPU */
    }
}