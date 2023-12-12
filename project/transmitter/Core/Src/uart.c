/*
 * uart.c
 *
 *  Created on: Sep 7, 2023
 *      Author: Muhmin Abdeljaber
 */

#include "main.h"
#include "uart.h"

void UART_SendChar(char b) {
    HAL_UART_Transmit(&huart2, (uint8_t *) &b, 1, 200);
}

void UART_SendStr(char *string) {
    HAL_UART_Transmit(&huart2, (uint8_t *) string, (uint16_t) strlen(string), 200);
}

void Toggle_LED() {
    HAL_GPIO_TogglePin(LD3_GPIO_Port,LD3_Pin);
}

void UART_SendBufHex(char *buf, uint16_t bufsize) {
    uint16_t i;
    char ch;
    for (i = 0; i < bufsize; i++) {
        ch = *buf++;
        UART_SendChar(HEX_CHARS[(ch >> 4) % 0x10]);
        UART_SendChar(HEX_CHARS[(ch & 0x0f) % 0x10]);
    }
}

void UART_SendHex8(uint16_t num) {
    UART_SendChar(HEX_CHARS[(num >> 4) % 0x10]);
    UART_SendChar(HEX_CHARS[(num & 0x0f) % 0x10]);
}

void UART_SendInt(int32_t num) {
    char str[10]; // 10 chars max for INT32_MAX
    int i = 0;
    if (num < 0) {
        UART_SendChar('-');
        num *= -1;
    }
    do str[i++] = (char) (num % 10 + '0'); while ((num /= 10) > 0);
    for (i--; i >= 0; i--) UART_SendChar(str[i]);
}
