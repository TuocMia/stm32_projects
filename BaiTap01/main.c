#include <stdint.h>

#define RCC_APB2ENR   (*((volatile uint32_t *)0x40021018))
#define GPIOC_CRH     (*((volatile uint32_t *)0x40011004))
#define GPIOC_ODR     (*((volatile uint32_t *)0x4001100C))

int main(void);

void Reset_Handler(void) {
    main();
    while(1);
}

__attribute__((section(".isr_vector")))
uint32_t *vector_table[] = {
    (uint32_t *)0x20005000,
    (uint32_t *)Reset_Handler,
};

int main(void) {
    RCC_APB2ENR |= (1 << 4);
    
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);

    while (1) {
        GPIOC_ODR ^= (1 << 13);
        for (volatile uint32_t i = 0; i < 200000; i++);
    }
    return 0;
}