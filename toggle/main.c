#include <stdint.h>

#define RCC_BASE    0x40021000
#define GPIOA_BASE  0x40010800

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define GPIOA_CRL   (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_CRH   (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_IDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08))

int main(void){
    RCC_APB2ENR |= (1 << 2);
    GPIOA_CRL = 0x88888888;
    GPIOA_CRH = 0x22222222;
    GPIOA_ODR |= 0xFF;

    while(1){
        uint32_t input = GPIOA_IDR & 0xFF;
        uint32_t invert = (~input) & 0xFF;
        GPIOA_ODR = (GPIOA_ODR & 0x000000FF) | (invert << 8);
    }
}