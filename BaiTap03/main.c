#include <stdint.h>

#define RCC_APB2ENR   (*((volatile uint32_t *)0x40021018))
#define GPIOA_CRL     (*((volatile uint32_t *)0x40010800))
#define GPIOA_CRH     (*((volatile uint32_t *)0x40010804))
#define GPIOA_IDR     (*((volatile uint32_t *)0x40010808))
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

int main(void) {
    RCC_APB2ENR |= (1 << 2);
    
    GPIOA_CRL = 0x88888888;
    
    GPIOA_CRH &= 0x0FF00000;

    GPIOA_CRH |= 0x30033333;
    
    GPIOA_ODR |= 0x00FF;

    while (1) {
        uint32_t input_data = GPIOA_IDR & 0xFF;
        
        uint32_t output_data = (~input_data) & 0xFF;
        
        GPIOA_ODR = (output_data << 8) | 0x00FF;
        
        for(volatile uint32_t i = 0; i < 50000; i++);
    }
    return 0;
}