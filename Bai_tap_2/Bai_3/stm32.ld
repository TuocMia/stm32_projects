/*
 * Linker script cho STM32F103.
 *
 * Xác định:
 * - Vị trí và kích thước vùng Flash.
 * - Vị trí và kích thước vùng RAM.
 * - Cách sắp xếp các section của chương trình vào bộ nhớ.
 */

ENTRY(Reset_Handler)

/*
 * Đặt stack pointer ở cuối vùng RAM.
 */
_estack = ORIGIN(RAM) + LENGTH(RAM);


MEMORY
{
    /*
     * STM32F103 sử dụng:
     * - Flash bắt đầu tại 0x08000000, dung lượng 64 KB.
     * - SRAM bắt đầu tại 0x20000000, dung lượng 20 KB.
     */
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}


SECTIONS
{
    /*
     * Bảng vector ngắt.
     *
     * Được đặt tại đầu vùng Flash để MCU có thể sử dụng
     * bảng vector khi khởi động và xử lý ngắt.
     */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH


    /*
     * Section .text chứa mã chương trình và .rodata
     * chứa các dữ liệu chỉ đọc.
     *
     * Hai section này được lưu trong Flash.
     */
    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text*)

        *(.rodata)
        *(.rodata*)

        . = ALIGN(4);
        _etext = .;
    } > FLASH


    /*
     * _sidata chứa địa chỉ vùng dữ liệu khởi tạo
     * được lưu trong Flash.
     */
    _sidata = LOADADDR(.data);


    /*
     * Section .data chứa các biến toàn cục/static
     * đã được khởi tạo giá trị.
     *
     * Khi khởi động:
     * - Giá trị ban đầu được lưu trong Flash.
     * - Sau đó được startup code copy vào RAM để chương trình sử dụng.
     */
    .data :
    {
        . = ALIGN(4);

        _sdata = .;
        *(.data)
        *(.data*)

        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH


    /*
     * Section .bss chứa các biến toàn cục/static
     * chưa được khởi tạo hoặc được khởi tạo bằng 0.
     *
     * Các biến trong .bss được startup code đưa về 0
     * trước khi gọi hàm main().
     */
    .bss :
    {
        . = ALIGN(4);

        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > RAM


    /*
     * Địa chỉ cuối của vùng dữ liệu chương trình trong RAM.
     * Symbol 'end' được sử dụng bởi một số thành phần của thư viện C.
     */
    . = ALIGN(4);
    _end = .;
    end = .;
}