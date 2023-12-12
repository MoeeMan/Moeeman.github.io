/*
 * motor.c
 *
 *  Created on: Aug 31, 2023
 *      Author: Muhmin Abdeljaber
 */


#include "main.h"
#include "uart.h"

#define DEFAULT_X			2000
#define DEFAULT_Y			2000
#define THRESHOLD_AXIS		30
#define MAX_PWM				(htim1.Init.Period)
#define MIN_PWM_PERC		.1
#define MAX_THROTTLE		4038

uint32_t throttleToPwm(uint32_t adcVal);
void brake(void);
void throttle(uint32_t pwmY);
uint32_t calcDelay(uint32_t pwmY);
uint32_t calcStopDistance(uint32_t pwmY);

uint32_t newMaxPwm;
uint32_t minPwm;
int state;
uint32_t lastDirDelay;

typedef enum {
	BRAKE = 0,
	FORWARD = 1,
	BACKWARD = 2
} Direction;

void motor_task(void *param)
{
	BaseType_t status;
	QUEUE_DATA q_data;
	uint32_t posX;
	uint32_t posY;
	uint16_t median_dist;
	uint32_t throttle_limit;
	uint8_t enable_ultrasonic;
	state = BRAKE;
	int stop = 0;
	uint32_t dirDelay;
	uint32_t maxDirDelay;
	uint32_t pwmY;
	uint8_t toggleHeadlights;
	uint32_t stopDistance;

	while(1)
	{
		status = xQueueReceive(eventQueue, &q_data, portMAX_DELAY);
		if(status == pdTRUE)
		{
			if(q_data.eventType == JOYSTICK)
			{
				posY = q_data.data[0];
				posX = q_data.data[1];
				throttle_limit = q_data.data[2];
				enable_ultrasonic = q_data.data[3];
				toggleHeadlights = q_data.data[4];

				if(toggleHeadlights)
					HAL_GPIO_TogglePin(HeadLights_GPIO_Port, HeadLights_Pin);

				stop = enable_ultrasonic ? stop : 0;

				newMaxPwm = ((double)throttle_limit / MAX_THROTTLE) * MAX_PWM;
				minPwm = newMaxPwm * MIN_PWM_PERC;


				HAL_GPIO_WritePin(PWMB_GPIO_Port, PWMB_Pin, GPIO_PIN_SET);
				if(posX >= DEFAULT_X+THRESHOLD_AXIS)
				{
					//turn_right
					HAL_GPIO_WritePin(FrontMotor_IN1_GPIO_Port, FrontMotor_IN1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(FrontMotor_IN2_GPIO_Port, FrontMotor_IN2_Pin, GPIO_PIN_RESET);
				}
				else if(posX <= DEFAULT_X-THRESHOLD_AXIS)
				{
					//turn_left
					HAL_GPIO_WritePin(FrontMotor_IN1_GPIO_Port, FrontMotor_IN1_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(FrontMotor_IN2_GPIO_Port, FrontMotor_IN2_Pin, GPIO_PIN_SET);
				}
				else if(posX < DEFAULT_X+THRESHOLD_AXIS || posX > DEFAULT_X-THRESHOLD_AXIS)
				{
					//no_turn
					HAL_GPIO_WritePin(FrontMotor_IN1_GPIO_Port, FrontMotor_IN1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(FrontMotor_IN2_GPIO_Port, FrontMotor_IN2_Pin, GPIO_PIN_SET);
				}

				//FORWARD
				if((posY <= DEFAULT_Y-THRESHOLD_AXIS) && !stop)
				{
					dirDelay = HAL_GetTick() - lastDirDelay;
					maxDirDelay = calcDelay(pwmY);
					if(state == BACKWARD && (dirDelay < maxDirDelay))
					{
						state = BRAKE;
						brake();
						vTaskDelay(maxDirDelay - dirDelay);
					}
					else
					{
						state = FORWARD;
						pwmY = throttleToPwm(posY);
						throttle(pwmY);
					}
				}
				//BACKWARD
				else if(posY >= DEFAULT_Y+THRESHOLD_AXIS)
				{
					dirDelay = HAL_GetTick() - lastDirDelay;
					maxDirDelay = calcDelay(pwmY);
					if(state == FORWARD && (dirDelay < maxDirDelay))
					{
						state = BRAKE;
						brake();
						vTaskDelay(maxDirDelay - dirDelay);
					}
					else
					{
						state = BACKWARD;
						pwmY = throttleToPwm(posY);
						throttle(pwmY);
					}
				}
				//BRAKE
				else if(posY < DEFAULT_Y+THRESHOLD_AXIS || posY > DEFAULT_Y-THRESHOLD_AXIS)
				{
					brake();
				}
			}
			else if(q_data.eventType == ULTRASONIC && enable_ultrasonic)
			{
				stopDistance = calcStopDistance(pwmY);
				median_dist = q_data.data[0];
				if(median_dist > stopDistance && !stop)
				{
					stop = 0;
				}
				else if((median_dist <= stopDistance))
				{
					stop = 1;
				}
				//avoid flickering between red and green
				else if(median_dist > stopDistance+3 && stop)
				{
					stop = 0;
				}
			}
		}
	}
}

uint32_t throttleToPwm(uint32_t adcVal)
{
	int a = 4035 - (DEFAULT_Y+THRESHOLD_AXIS);	//a=2000
	double b = abs(adcVal - (DEFAULT_Y+THRESHOLD_AXIS));
	double c = (b / a) * (1-MIN_PWM_PERC);
	uint32_t newPwm = minPwm + (newMaxPwm * c);

	return newPwm;
}

uint32_t calcDelay(uint32_t pwmY)
{
	uint32_t delay = 100;
	if(pwmY > MAX_PWM/2)
		delay += ((pwmY/10)*4);

	return delay;
}

uint32_t calcStopDistance(uint32_t pwmY)
{
	uint32_t distance = 10;
	distance += pwmY/8;

	return distance;
}

void brake(void)
{
	HAL_GPIO_WritePin(BackMotor_IN1_GPIO_Port, BackMotor_IN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BackMotor_IN2_GPIO_Port, BackMotor_IN2_Pin, GPIO_PIN_RESET);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

void throttle(uint32_t pwmY)
{
	if(state == FORWARD)
	{
		HAL_GPIO_WritePin(BackMotor_IN1_GPIO_Port, BackMotor_IN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BackMotor_IN2_GPIO_Port, BackMotor_IN2_Pin, GPIO_PIN_SET);
	}
	else if(state == BACKWARD)
	{
		HAL_GPIO_WritePin(BackMotor_IN1_GPIO_Port, BackMotor_IN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BackMotor_IN2_GPIO_Port, BackMotor_IN2_Pin, GPIO_PIN_RESET);
	}

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwmY);
	lastDirDelay = HAL_GetTick();
}

