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

static void init_receiver(void);

uint8_t nRF24_payload[8];
uint8_t payload_length = 8;

void receive_task(void *param)
{
	nRF24_RXResult pipe;
	uint32_t VRx;
	uint32_t VRy;
	uint32_t throttle_limit;
	uint32_t enable_ultrasonic;
	uint32_t timeoutTick;
	uint8_t toggleHeadlights;

	// RX/TX disabled
	nRF24_CE_L();

#if (DEBUG_UART)
	UART_SendStr("\r\nSTM32L432KC is online (Receiver)\r\n");

	// Configure the nRF24L01+
	UART_SendStr("nRF24L01+ check: ");
	while (!nRF24_Check())
	{
		UART_SendStr("FAIL\r\n");
		Toggle_LED();
		HAL_Delay(50);
	}
	UART_SendStr("OK\r\n");
#else
	while (!nRF24_Check())
	{
		Toggle_LED();
		HAL_Delay(50);
	}
#endif

	// Initialize the nRF24L01 to its default state
	init_receiver();

	while (1) {
		//
		// Constantly poll the status of the RX FIFO and get a payload if FIFO is not empty
		//
		// This is far from best solution, but it's ok for testing purposes
		// More smart way is to use the IRQ pin :)
		//
		if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
			// Get a payload from the transceiver
			pipe = nRF24_ReadPayload(nRF24_payload, &payload_length);

			// Clear all pending IRQ flags
			nRF24_ClearIRQFlags();

			QUEUE_DATA q_data;
			VRy = 0;
			VRx = 0;
			throttle_limit = 0;

			VRy |= nRF24_payload[0] << 8;
			VRy |= nRF24_payload[1];
			VRx |= nRF24_payload[2] << 8;
			VRx |= nRF24_payload[3];

			throttle_limit |= nRF24_payload[4] << 8;
			throttle_limit |= nRF24_payload[5];

			enable_ultrasonic = nRF24_payload[6];

			toggleHeadlights = nRF24_payload[7];

			q_data.eventType = JOYSTICK;
			q_data.data[0] = VRy;
			q_data.data[1] = VRx;
			q_data.data[2] = throttle_limit;
			q_data.data[3] = enable_ultrasonic;
			q_data.data[4] = toggleHeadlights;

			if(!enable_ultrasonic)
				vTaskSuspend(ultrasonic_task_handle);
			else
				vTaskResume(ultrasonic_task_handle);

			xQueueSend(eventQueue, &q_data, portMAX_DELAY);

			timeoutTick = HAL_GetTick();

#if (DEBUG_UART)
			// Print a payload contents to UART
			UART_SendStr("RCV PIPE#");
			UART_SendInt(pipe);
			UART_SendStr("	VRy: ");
			UART_SendInt(VRy);
			UART_SendStr(" VRx: ");
			UART_SendInt(VRx);
			UART_SendStr(" Throttle_limit: ");
			UART_SendInt(throttle_limit);
			UART_SendStr(" enable_ultrasonic: ");
			UART_SendInt(enable_ultrasonic);
			UART_SendStr("\r\n");
#endif
		}
		else
		{
			//Receiver hasn't gotten any data for 500ms so stop all motors
			if(timeoutTick + 500 < HAL_GetTick())
			{
				HAL_GPIO_WritePin(FrontMotor_IN1_GPIO_Port, FrontMotor_IN1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(FrontMotor_IN2_GPIO_Port, FrontMotor_IN2_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(BackMotor_IN1_GPIO_Port, BackMotor_IN1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(BackMotor_IN2_GPIO_Port, BackMotor_IN2_Pin, GPIO_PIN_SET);

				UART_SendStr("NOT RESPONDING");
				UART_SendStr("\r\n");
			}
		}
	}
}

static void init_receiver(void)
{
	nRF24_Init();

	static const uint8_t nRF24_ADDR[] = { 0xE7, 0x1C, 0xE3 }; // the TX address
	nRF24_DisableAA(0xFF); // disable ShockBurst
	nRF24_SetRFChannel(115); // set RF channel to 2515MHz
	nRF24_SetDataRate(nRF24_DR_250kbps); // 250kbit/s data rate (minimum possible, to increase reception reliability)
	nRF24_SetCRCScheme(nRF24_CRC_2byte); // 2-byte CRC scheme
	nRF24_SetAddrWidth(3); // address width is 3 bytes
	nRF24_SetAddr(nRF24_PIPE1, nRF24_ADDR); // program address for RX pipe #1
	nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_OFF, payload_length); // Auto-ACK: disabled, payload length: 5 bytes
	nRF24_SetOperationalMode(nRF24_MODE_RX); // switch transceiver to the RX mode
	nRF24_SetPowerMode(nRF24_PWR_UP); // wake-up transceiver (in case if it sleeping)

	// Put the transceiver to the RX mode
	nRF24_CE_H();
}
