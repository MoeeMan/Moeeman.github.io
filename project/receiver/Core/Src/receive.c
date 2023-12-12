/*
 * transmit.c
 *
 *  Created on: Aug 29, 2023
 *      Author: Muhmin Abdeljaber
 */

#include "main.h"
#include "nrf24.h"


//create struct for joystick readings
typedef struct{
	uint32_t VR[2];
}JoystickPos;


nRF24_RXResult rx_res;

void receive_task(void *param)
{
	//Receiver, one logic address (pipe#1), without ShockBurst

	uint8_t ADDR[] = { 'n', 'R', 'F', '2', '4' }; // the address for RX pipe
	nRF24_SetRFChannel(90); // set RF channel to 2490MHz
	nRF24_SetDataRate(nRF24_DR_2Mbps); // 2Mbit/s data rate
	nRF24_SetCRCScheme(nRF24_CRC_1byte); // 1-byte CRC scheme
	nRF24_SetAddrWidth(5); // address width is 5 bytes
	nRF24_SetAddr(nRF24_PIPE1, ADDR); // program pipe address
	nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, 10); // enable RX pipe#1 with Auto-ACK: enabled, payload length: 10 bytes
	nRF24_SetTXPower(nRF24_TXPWR_0dBm); // configure TX power for Auto-ACK, good choice - same power level as on transmitter
	nRF24_SetOperationalMode(nRF24_MODE_RX); // switch transceiver to the RX mode
	nRF24_SetPowerMode(nRF24_PWR_UP); // wake-up transceiver (in case if it sleeping)

	// then pull CE pin to HIGH, and the nRF24 will start a receive...
	uint32_t nRF24_payload[2];
	uint8_t payload_length;
	uint8_t pipe; // pipe number
	nRF24_CE_H(); // start receiving

	while(1)
	{
		// constantly poll the status of RX FIFO...
		if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
			// the RX FIFO have some data, take a note what nRF24 can hold up to three payloads of 32 bytes...
			pipe = nRF24_ReadPayload(nRF24_payload, &payload_length); // read a payload to buffer
			nRF24_ClearIRQFlags(); // clear any pending IRQ bits
			// now the nRF24_payload buffer holds received data
			// payload_length variable holds a length of received data
			// pipe variable holds a number of the pipe which has received the data
			// ... do something with received data ...
			printf("%d\n",pipe);
			JoystickPos pos;
			pos.VR[0] = nRF24_payload[0];
			pos.VR[1] = nRF24_payload[1];
			xQueueSend(q_joystick_pos, &pos, portMAX_DELAY);
		}
	}
}
