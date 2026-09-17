#include "main.h"
#include <stdio.h>
#include <string.h>

/*
 * Khai báo các handle dùng để quản lý các ngoại vi thông qua thư viện HAL.
 * hadc1: quản lý ADC1.
 * huart1: quản lý USART1.
 */
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;

/*
 * Khai báo nguyên mẫu các hàm khởi tạo ngoại vi.
 * Các hàm này được đặt static vì chỉ được sử dụng trong file main.c.
 */
static void ADC1_Init(void);
static void USART1_Init(void);


int main(void)
{
    /*
     * Khởi tạo thư viện HAL.
     * Hàm này thiết lập các thành phần cơ bản của HAL,
     * trong đó có SysTick để tạo bộ đếm thời gian cho HAL_Delay().
     */
    HAL_Init();


    /* ================= GPIO PC13 ================= */

    /*
     * Kích hoạt clock cho GPIO Port C.
     * PC13 được sử dụng để điều khiển LED trên board,
     * giúp quan sát trạng thái hoạt động của chương trình.
     */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*
     * Cấu hình chân PC13:
     * - GPIO_PIN_13: sử dụng chân PC13.
     * - GPIO_MODE_OUTPUT_PP: ngõ ra Push-Pull.
     * - GPIO_SPEED_FREQ_LOW: tốc độ chuyển mức thấp,
     *   phù hợp với việc điều khiển LED.
     */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


    /* ================= Khởi tạo ADC và USART ================= */

    /*
     * Khởi tạo ADC1 để đọc tín hiệu analog từ chân PA0
     * và USART1 để truyền kết quả ADC lên máy tính.
     */
    ADC1_Init();
    USART1_Init();

    /*
     * Thực hiện hiệu chỉnh ADC trước khi bắt đầu chuyển đổi.
     * Việc calibration giúp cải thiện độ chính xác của phép đo ADC.
     */
    HAL_ADCEx_Calibration_Start(&hadc1);

    /*
     * Bộ đệm dùng để lưu chuỗi dữ liệu trước khi truyền qua UART.
     */
    char msg[64];


    while (1)
    {
        /*
         * Đảo trạng thái LED PC13 sau mỗi chu kỳ.
         * LED được sử dụng như một tín hiệu trực quan cho thấy
         * chương trình vẫn đang hoạt động.
         */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);


        /*
         * Bắt đầu một lần chuyển đổi ADC.
         * ADC sẽ chuyển đổi điện áp analog tại PA0
         * thành giá trị số 12-bit.
         */
        HAL_ADC_Start(&hadc1);


        /*
         * Chờ ADC hoàn thành quá trình chuyển đổi.
         * Timeout = 100 ms để tránh chương trình bị chờ vô hạn
         * nếu quá trình chuyển đổi không hoàn thành.
         */
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            /*
             * Đọc kết quả chuyển đổi ADC.
             *
             * STM32F103 sử dụng ADC 12-bit nên giá trị trả về
             * nằm trong khoảng:
             *     0 --> 4095
             */
            uint32_t adc_value = HAL_ADC_GetValue(&hadc1);


            /*
             * Tính điện áp đầu vào từ giá trị ADC.
             *
             * Với ADC 12-bit và điện áp tham chiếu 3.3 V:
             *
             *     V = ADC_value / 4095 * 3.3 V
             *
             * Để tránh sử dụng số thực, điện áp được tính
             * theo đơn vị mV:
             *
             *     V(mV) = ADC_value * 3300 / 4095
             */
            uint32_t voltage_mv =
                (adc_value * 3300UL) / 4095UL;


            /*
             * Chuyển giá trị ADC và điện áp thành chuỗi ký tự
             * để truyền qua UART.
             *
             * Ví dụ:
             *     ADC: 2048 | Voltage: 1.650 V
             *
             * %03lu đảm bảo phần thập phân luôn có 3 chữ số.
             */
            int len = snprintf(
                msg,
                sizeof(msg),
                "ADC: %lu | Voltage: %lu.%03lu V\r\n",
                adc_value,
                voltage_mv / 1000UL,
                voltage_mv % 1000UL
            );


            /*
             * Truyền chuỗi kết quả qua USART1.
             *
             * PA9 được cấu hình là USART1_TX,
             * kết nối với RX của USB-UART (CP2102)
             * để đưa dữ liệu lên máy tính.
             */
            HAL_UART_Transmit(
                &huart1,
                (uint8_t *)msg,
                len,
                100
            );
        }


        /*
         * Dừng ADC sau khi hoàn thành một lần đo.
         * Lần đo tiếp theo sẽ được bắt đầu lại bằng HAL_ADC_Start().
         */
        HAL_ADC_Stop(&hadc1);


        /*
         * Chờ 1000 ms trước khi thực hiện lần đo tiếp theo.
         * Do đó dữ liệu ADC và điện áp được gửi lên máy tính
         * với chu kỳ khoảng 1 giây.
         */
        HAL_Delay(1000);
    }
}


/* ============================================================
 *                           ADC1
 * ============================================================ */

static void ADC1_Init(void)
{
    /*
     * Biến cấu hình cho kênh ADC.
     */
    ADC_ChannelConfTypeDef sConfig = {0};


    /*
     * Kích hoạt clock cho ADC1 và GPIO Port A.
     * PA0 thuộc GPIOA và được sử dụng làm đầu vào ADC.
     */
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};


    /*
     * Cấu hình PA0 làm ngõ vào analog.
     *
     * PA0 tương ứng với ADC1_IN0 trên STM32F103.
     * Tín hiệu analog từ biến trở/cảm biến được đưa vào chân này.
     */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /*
     * Chọn ADC1 làm thiết bị ADC được cấu hình.
     */
    hadc1.Instance = ADC1;


    /*
     * Cấu hình ADC1:
     *
     * - ScanConvMode = DISABLE:
     *   Chỉ sử dụng một kênh ADC nên không cần quét nhiều kênh.
     *
     * - ContinuousConvMode = DISABLE:
     *   Không chuyển đổi liên tục.
     *   Mỗi lần đo sẽ được bắt đầu bằng HAL_ADC_Start().
     *
     * - ExternalTrigConv = ADC_SOFTWARE_START:
     *   ADC được kích hoạt bằng phần mềm.
     *
     * - DataAlign = RIGHT:
     *   Kết quả ADC được căn phải.
     *
     * - NbrOfConversion = 1:
     *   Mỗi lần chỉ thực hiện một phép chuyển đổi.
     */
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;

    /*
     * Áp dụng các cấu hình ADC1 ở trên.
     */
    HAL_ADC_Init(&hadc1);


    /*
     * Cấu hình kênh ADC được sử dụng:
     *
     * - ADC_CHANNEL_0: kênh ADC số 0, tương ứng PA0.
     * - ADC_REGULAR_RANK_1: vị trí đầu tiên trong chuỗi chuyển đổi.
     * - ADC_SAMPLETIME_239CYCLES_5: thời gian lấy mẫu dài,
     *   giúp ADC có đủ thời gian lấy tín hiệu analog.
     */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    /*
     * Áp dụng cấu hình kênh ADC.
     */
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}


/* ============================================================
 *                          USART1
 * ============================================================ */

static void USART1_Init(void)
{
    /*
     * Kích hoạt clock cho USART1 và GPIO Port A.
     * USART1 sử dụng các chân PA9 và PA10.
     */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};


    /*
     * Cấu hình PA9 làm chân truyền dữ liệu USART1_TX.
     *
     * PA9 -> TX của STM32
     * PA9 -> RX của USB-UART (CP2102)
     */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /*
     * Cấu hình PA10 làm chân nhận dữ liệu USART1_RX.
     *
     * Trong bài này chủ yếu truyền dữ liệu từ STM32
     * lên máy tính nên PA10 không được sử dụng trong luồng dữ liệu chính,
     * nhưng vẫn được cấu hình để USART1 hoạt động ở chế độ TX/RX.
     */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /*
     * Chọn USART1 làm thiết bị UART được cấu hình.
     */
    huart1.Instance = USART1;


    /*
     * Cấu hình thông số truyền UART:
     *
     * - BaudRate = 9600 bit/s
     * - WordLength = 8 bit
     * - StopBits = 1 bit
     * - Parity = None
     * - TX/RX được bật
     * - Không sử dụng Hardware Flow Control
     * - Oversampling = 16
     *
     * Các thông số này phải được cấu hình giống nhau
     * ở phía máy tính/USB-UART.
     */
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;


    /*
     * Áp dụng cấu hình USART1.
     */
    HAL_UART_Init(&huart1);
}