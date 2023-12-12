/*
 * uart.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Muhmin Abdeljaber
 */

#ifndef INC_UART_H_
#define INC_UART_H_


#define HEX_CHARS      "0123456789ABCDEF"

void UART_SendChar(char b);
void UART_SendStr(char *string);
void Toggle_LED();
void UART_SendBufHex(char *buf, uint16_t bufsize);
void UART_SendHex8(uint16_t num);
void UART_SendInt(int32_t num);

#endif /* INC_UART_H_ */
