#include "button.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"


void Button_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Bat clock cho GPIOA va AFIO */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_AFIO,
        ENABLE
    );

    /* Cau hinh PA0 lam Input Pull-Up */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(
        GPIOA,
        &GPIO_InitStructure
    );

    /*
     * Gan chan PA0 vao EXTI Line0
     */
    GPIO_EXTILineConfig(
        GPIO_PortSourceGPIOA,
        GPIO_PinSource0
    );

    /*
     * Cau hinh EXTI0:
     * - Che do Interrupt
     * - Kich hoat khi co canh xuong
     */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);

    /*
     * STM32F103 co 4 bit priority.
     * Su dung toan bo 4 bit cho Preemption Priority.
     */
    NVIC_PriorityGroupConfig(
        NVIC_PriorityGroup_4
    );

    /*
     * Cau hinh NVIC cho EXTI0.
     *
     * Priority = 5 de ISR co the su dung
     * cac ham FreeRTOS co duoi FromISR.
     */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    /* Xoa co ngat cu truoc khi bat NVIC. */
    EXTI_ClearITPendingBit(EXTI_Line0);

    NVIC_Init(&NVIC_InitStructure);
}