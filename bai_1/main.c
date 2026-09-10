#include <stdint.h>

#define RCC_BASE    0x40021000
#define GPIOC_BASE  0x40011000

#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE   + 0x18))
#define GPIOC_CRH    (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR    (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#define DELAY_COUNT 1000000

void delay(volatile uint32_t time)
{
    while (time > 0) {
        time = time - 1;
    }
}

int main(void) {
    RCC_APB2ENR |= (1 << 4);        // Enable clock for GPIOC
    GPIOC_CRH &= ~(0xF << 20);      // Clear old config for PC13
    GPIOC_CRH |=  (0x2 << 20);      // Output push-pull, 2MHz

    while (1) {
        GPIOC_ODR ^= (1 << 13);     // Toggle PC13
        delay(DELAY_COUNT);
    }
}
