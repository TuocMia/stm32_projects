#include <stdint.h>

#define RCC_BASE    0x40021000
#define GPIOA_BASE  0x40010800

#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define GPIOA_CRL   (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_IDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08))

#define BTN_PIN     0
#define LED_PIN     1

void delay(volatile uint32_t time)
{
    while (time > 0) {
        time = time - 1;
    }
}

int main(void){
    RCC_APB2ENR |= (1 << 2);
    GPIOA_CRL = 0x00000028;
    GPIOA_ODR |= (1 << BTN_PIN);

    int old_state = 0;

    while(1){
        int new_state = (GPIOA_IDR >> BTN_PIN) & 1;
        if (new_state == 0 && old_state == 1) {
            GPIOA_ODR ^= (1 << LED_PIN);
        }
        old_state = new_state;
        delay(100000);
    }
}