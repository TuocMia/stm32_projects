.syntax unified
.cpu cortex-m3
.thumb

.global _start
.global Reset_Handler

.section .isr_vector, "a"
_start:
    .word 0x20005000
    .word Reset_Handler

.section .text
.thumb_func
Reset_Handler:
    bl main
loop:
    b loop
