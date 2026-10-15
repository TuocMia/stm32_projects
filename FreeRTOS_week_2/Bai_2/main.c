#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "button.h"

#include <stdint.h>
#include <stdio.h>

QueueHandle_t xButtonCountQueue = NULL; /* Khai bao queue */

void USART1_Config(void) /* Cau hinh UART */
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd( /* Bat Clock GPIOA & UART */
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,
        ENABLE
    );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; /* Alternate Function_Push-Pull */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void UART_SendString(const char *str) /* Truyen chuoi~ qua UART */
{
    while (*str != '\0') /* Chua het chuoi thi tiep tuc truyen ky tu */
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) /* TXE = Transmit Data Register Empty */
        {
        }

        USART_SendData(USART1, (uint8_t)*str); /* Gui ky tu */
        str++; /* Dua tro toi ky tu tiep theo */
    }

    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) /* TC = Transmission Complete*/
    {
    }
}

void uart_task(void *task_parameter) /* Nhan du lieu tu Queue -> UART */
{
    uint32_t buttonCount;
    char txBuffer[48];

    (void)task_parameter;

    USART1_Config();
    UART_SendString("UART Ready\r\n");

    Button_EXTI_Init(); /* Enable nut nhan EXTI */

    for (;;)
    {
        if (xQueueReceive( /* Cho` du lieu */
                xButtonCountQueue,
                &buttonCount,
                portMAX_DELAY
            ) == pdPASS)
        {
            snprintf( /* In ra ket qua */
                txBuffer,
                sizeof(txBuffer),
                "Nut nhan: %lu\r\n",
                (unsigned long)buttonCount
            );

            UART_SendString(txBuffer);
        }
    }
}

int main(void)
{
    xButtonCountQueue = xQueueCreate(10, sizeof(uint32_t)); /* Tao Queue */

    if (xButtonCountQueue == NULL)
    {
        while (1) {}
    }

    if (xTaskCreate( /* Tao task UART */
            uart_task,
            "UART",
            384,
            NULL,
            2,
            NULL
        ) != pdPASS)
    {
        while (1) {}
    }

    vTaskStartScheduler(); /* Khoi dong freertos */

    while (1) {}
}
