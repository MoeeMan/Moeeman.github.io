/*
 * transmit.c
 *
 *  Created on: Aug 31, 2023
 *      Author: Muhmin Abdeljaber
 */

#include "main.h"
#include "support.h"
#include "nrf24.h"
#include "uart.h"

static void init_transmitter(void);

nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length);

uint8_t toggleHeadlights;

void transmit_task(void *param)
{
	nRF24_TXResult tx_res;
	uint8_t nRF24_payload[8];
	uint8_t payload_length = 8;
	uint8_t enable_ultrasonic = 0;

#if (DEBUG_UART)
	UART_SendStr("\r\nSTM32L432KC is online (Transmitter)\r\n");

	// RX/TX disabled
	nRF24_CE_L();

	// Configure the nRF24L01+
	UART_SendStr("nRF24L01+ check: ");
	while (!nRF24_Check()) {
		UART_SendStr("FAIL\r\n");
			Toggle_LED();
			HAL_Delay(50);
	}
	UART_SendStr("OK\r\n");
#endif

	// Initialize the nRF24L01 to its default state
	init_transmitter();

	while(1)
	{
		//Left joystick y-axis
		nRF24_payload[0] = (VR[0] >> 8) & 0xFF;
		nRF24_payload[1] = VR[0] & 0xFF;

		//Right joystick x-axis
		nRF24_payload[2] = (VR[1] >> 8) & 0xFF;
		nRF24_payload[3] = VR[1] & 0xFF;

		//Potentiometer
		nRF24_payload[4] = (VR[2] >> 8) & 0xFF;
		nRF24_payload[5] = VR[2] & 0xFF;


		enable_ultrasonic = HAL_GPIO_ReadPin(Switch_GPIO_Port, Switch_Pin) ? 0 : 1;

		//Switch
		nRF24_payload[6] = enable_ultrasonic;

		//Left Joystick Button Press
		nRF24_payload[7] = toggleHeadlights;
		if(toggleHeadlights)
		{
			UART_SendStr("TOGGLE");
			UART_SendStr("\r\n");
			toggleHeadlights = 0;
		}

		// the nRF24 is ready for transmission, upload a payload, then pull CE pin to HIGH and it will transmit a packet...
		tx_res = nRF24_TransmitPacket(nRF24_payload,payload_length);

#if (DEBUG_UART)
		// Print a payload
		UART_SendStr("PAYLOAD:>");
		UART_SendBufHex((char *)nRF24_payload, payload_length);
		UART_SendStr("< ... TX: ");

		switch (tx_res) {
			case nRF24_TX_SUCCESS:
				UART_SendStr("OK");
				break;
			case nRF24_TX_TIMEOUT:
				UART_SendStr("TIMEOUT");
				break;
			case nRF24_TX_MAXRT:
				UART_SendStr("MAX RETRANSMIT");
				break;
			default:
				UART_SendStr("ERROR");
				break;
		}
		UART_SendStr("\r\n");
#endif

		HAL_Delay(100);
	}
}

nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length)
{
	uint8_t status;


	nRF24_CE_L(); // Deassert the CE pin (in case if it still high)
	nRF24_WritePayload(pBuf, length); // transfer payload data to transceiver
	nRF24_CE_H(); // assert CE pin (transmission starts)

	while (1) {
		status = nRF24_GetStatus();
		if (status & (nRF24_FLAG_TX_DS | nRF24_FLAG_MAX_RT)) {
			// transmission ended, exit loop
			break;
		}
	}
	nRF24_CE_L(); // de-assert CE pin (nRF24 goes to StandBy-I mode)

	// Check the flags in STATUS register
	UART_SendStr("[");
	UART_SendHex8(status);
	UART_SendStr("] ");

	nRF24_ClearIRQFlags(); // clear any pending IRQ flags
	if (status & nRF24_FLAG_MAX_RT) {
		// Auto retransmit counter exceeds the programmed maximum limit (payload in FIFO is not removed)
		// Also the software can flush the TX FIFO here...
		return nRF24_TX_MAXRT;
	}
	if (status & nRF24_FLAG_TX_DS) {
		// Successful transmission
		return nRF24_TX_SUCCESS;
	}

	// Some banana happens, a payload remains in the TX FIFO, flush it
	nRF24_FlushTX();

	// In fact that should not happen
	return nRF24_TX_ERROR;
}

static void init_transmitter(void)
{
	// Initialize the nRF24L01 to its default state
	nRF24_Init();

	static const uint8_t nRF24_ADDR[] = { 0xE7, 0x1C, 0xE3 }; // the TX address
	nRF24_DisableAA(0xFF); // disable ShockBurst
	nRF24_SetRFChannel(115); // set RF channel to 2515MHz
	nRF24_SetDataRate(nRF24_DR_250kbps); // 250kbit/s data rate (minimum possible, to increase reception reliability)
	nRF24_SetCRCScheme(nRF24_CRC_2byte); // 2-byte CRC scheme
	nRF24_SetAddrWidth(3); // address width is 3 bytes
	nRF24_SetTXPower(nRF24_TXPWR_12dBm); // configure TX power (max)
	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR); // program TX address
	nRF24_SetOperationalMode(nRF24_MODE_TX); // switch transceiver to the TX mode
	nRF24_ClearIRQFlags(); // Clear any pending IRQ flags
	nRF24_SetPowerMode(nRF24_PWR_UP); // wake-up transceiver (in case if it sleeping)
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	toggleHeadlights = 1;
}
