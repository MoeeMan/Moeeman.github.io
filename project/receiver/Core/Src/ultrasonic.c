/*
 * ultrasonic.c
 *
 *  Created on: Aug 31, 2023
 *      Author: Muhmin Abdeljaber
 */

#include "main.h"
#include "uart.h"

#define MAX_READINGS	5

#define CMP_SWAP(i, j) if (dist_readings[i] > dist_readings[j]) \
	{ int tmp = dist_readings[i]; dist_readings[i] = dist_readings[j]; dist_readings[j] = tmp; }

void delay_us(uint16_t delay);
void store_reading(uint16_t dist);

uint32_t t1;
uint32_t t2;
int speed;
double temp;
uint8_t firstReading = 1;
uint16_t firstDist;
int speedArray[100];
int idx=0;


void ultrasonic_task(void *param)
{
	uint32_t x1;
	uint32_t x2;
	uint16_t dist;

	while(1){
		HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
		delay_us(2);

		HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
		delay_us(10);
		HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);

		while(!HAL_GPIO_ReadPin(Echo_GPIO_Port, Echo_Pin))
		{
			//wait until echo goes high
			x1 = __HAL_TIM_GET_COUNTER(&htim2);
		}

		while(HAL_GPIO_ReadPin(Echo_GPIO_Port, Echo_Pin))
		{
			//wait until echo goes low
			x2 = __HAL_TIM_GET_COUNTER(&htim2);
		}

		//calculate distance
		dist = (x2-x1) * 0.034/2;

		store_reading(dist);
	}
}

void store_reading(uint16_t dist)
{
	static uint16_t dist_readings[MAX_READINGS];
	static int i = 0;
	uint16_t median_dist;
	QUEUE_DATA q_data;

	dist_readings[i] = dist;
	i++;

	//array full. Only works for 5 entries
	if(i == MAX_READINGS)
	{
		CMP_SWAP(0, 3); CMP_SWAP(1, 4); CMP_SWAP(0, 2); CMP_SWAP(1, 3);
		CMP_SWAP(0, 1); CMP_SWAP(2, 4); CMP_SWAP(1, 2); CMP_SWAP(3, 4);
		CMP_SWAP(2, 3);

		median_dist = dist_readings[MAX_READINGS/2];
		i=0;
		memset(dist_readings, 0, sizeof(dist_readings));

		q_data.eventType = ULTRASONIC;
		q_data.data[0] = median_dist;

		if(firstReading)
		{
			t1 = HAL_GetTick();
			firstDist = dist;
			firstReading = 0;
		}
		else
		{
			t2 = HAL_GetTick();
			temp = (int)(t2 - t1) / (double)1000;
			speed = (firstDist - dist) / temp;
			speedArray[idx] = speed;
			idx = (idx+1) % 100;
			firstReading = 1;
		}

		for(int i=0;i<idx;i++)
		{
			UART_SendInt(i);
			UART_SendStr(": ");
			UART_SendInt(speedArray[i]);
			UART_SendStr("\r\n");
		}

#if (DEBUG_UART)
		UART_SendStr("Distance (cm):");
		UART_SendInt(dist);
		UART_SendStr("\r\n");
#endif

		//send to front because its important. Dont block if queue is full
		xQueueSendToFront(eventQueue, &q_data, ( TickType_t ) 0);

		vTaskDelay(150);
	}
}


void delay_us(uint16_t delay)
{
	__HAL_TIM_SET_COUNTER(&htim2,0);
	while(__HAL_TIM_GET_COUNTER(&htim2) < delay);
}
