#include "stm32f10x.h"
#include "stm32f10x_exti.h"

#include "FreeRTOS.h"
#include "queue.h"

#include <stdint.h>


/*
 * Queue duoc tao trong main.c
 */
extern QueueHandle_t xButtonCountQueue;


/*
 * Bien dem so lan nhan nut
 */
static volatile uint32_t buttonCount = 0;


/*
 * Ham xu ly ngat EXTI0
 */
void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t countToSend;

    /*
     * Kiem tra EXTI Line0 co phat sinh ngat hay khong
     */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        /*
         * Tang so lan nhan nut
         */
        buttonCount++;
        countToSend = buttonCount;

        /*
         * Gui gia tri buttonCount vao Queue.
         * Ham nay duoc thiet ke rieng de goi trong ISR.
         */
        xQueueSendFromISR(
            xButtonCountQueue,
            &countToSend,
            &xHigherPriorityTaskWoken
        );

        /*
         * Xoa co pending cua EXTI0
         */
        EXTI_ClearITPendingBit(EXTI_Line0);

        /*
         * Neu Queue lam mot Task uu tien cao hon duoc danh thuc,
         * yeu cau chuyen ngu canh ngay sau khi thoat ISR.
         */
        portYIELD_FROM_ISR(
            xHigherPriorityTaskWoken
        );
    }
}