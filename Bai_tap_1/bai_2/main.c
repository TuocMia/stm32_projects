#include <stdint.h>

#define RCC_BASE    0x40021000
#define GPIOA_BASE   0x40010800

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define GPIOA_CRL   (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))

void delay(volatile int time)
{
    while (time > 0) {
        time = time - 1;
    }
}

int main(void) {
    RCC_APB2ENR |= (1 << 2);        // Enable clock for GPIOA
    GPIOA_CRL = 0x22222222;         // Output push-pull, 2MHz for PA0-7

    while(1) {
        for(int i = 0; i <= 7; i++) {
            GPIOA_ODR = (1 << i);
            delay(100000);
        }
        for(int i = 7; i > 1; i--){
            GPIOA_ODR = (1 << (i - 1));
            delay(100000);
        }
    }
}
