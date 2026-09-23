#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "gui.h"
#include "lcd.h"
#include "delay.h"

void RCC_Configuration(void);
void GPIO_Configuration(void);
void SPI1_Configuration(void);
static void LCD_TestScreen(void);

int main(void)
{
    RCC_Configuration();
    delay_init(72); // Initialize delay function with system clock of 72 MHz
    GPIO_Configuration();
    SPI1_Configuration();

    LCD_Init(); // Initialize the LCD
    LCD_TestScreen();
    
    while (1)
    {

    }
}

static void LCD_TestScreen(void)
{
    POINT_COLOR = BLACK;
    LCD_Fill(0, 0, LCD_W - 1, LCD_H - 1, BLACK);

    POINT_COLOR = RED;
    LCD_DrawFillRectangle(10, 10, 70, 70);
    LCD_DrawRectangle(10, 10, 70, 70);

    POINT_COLOR = GREEN;
    LCD_DrawFillRectangle(85, 10, 145, 70);
    LCD_DrawRectangle(85, 10, 145, 70);

    POINT_COLOR = BLUE;
    LCD_DrawFillRectangle(160, 10, 220, 70);
    LCD_DrawRectangle(160, 10, 220, 70);

    POINT_COLOR = YELLOW;
    LCD_DrawRectangle(20, 90, 220, 210);
    LCD_DrawLine(20, 90, 220, 210);
    LCD_DrawLine(20, 210, 220, 90);

    POINT_COLOR = CYAN;
    gui_circle(120, 150, CYAN, 35, 0);

    POINT_COLOR = WHITE;
    GUI_DrawPoint(120, 150, WHITE);
}

void RCC_Configuration(void)
{
  /* Enable clocks for GPIOA, GPIOB, and SPI1 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                            RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE); 
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; // SCK and MOSI
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternate function push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // MISO
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Input pull-up
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //LCD GPIO initialization
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9; // PB7 DC, PB8 RST, PB9 CS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // Output push-pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9); // Set LCD control pins high
}

void SPI1_Configuration(void)
{
    SPI_InitTypeDef SPI_InitStructure;

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE); // Enable SPI1
}