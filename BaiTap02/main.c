#include <stdint.h>

#define RCC_APB2ENR   (*((volatile uint32_t *)0x40021018))
#define GPIOA_CRL     (*((volatile uint32_t *)0x40010800))
#define GPIOA_ODR     (*((volatile uint32_t *)0x4001080C))

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

// Hàm trễ (Delay) mù
void delay(uint32_t time) {
    for (volatile uint32_t i = 0; i < time; i++);
}

int main(void) {
    RCC_APB2ENR |= (1 << 2);
    
    GPIOA_CRL = 0x33333333; 

    int8_t pos = 0;
    int8_t dir = 1;

    while (1) {
        GPIOA_ODR = (1 << pos);
        
        delay(300000);

        pos += dir;

        if (pos == 7) dir = -1;
        else if (pos == 0) dir = 1;
    }
    return 0;
}