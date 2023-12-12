/*
 * motor.c
 *
 *  Created on: Aug 30, 2023
 *      Author: Muhmin Abdeljaber
 */


#include "main.h"

void motor_task(void *param)
{
	BaseType_t status;
	JoystickPos pos;
	uint32_t posX;
	uint32_t posY;

	//keep stby on?
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	while(1)
	{
		status = xQueueReceive(q_joystick_pos, &pos, portMAX_DELAY);
		if(status == pdTRUE)
		{
			posX = pos.VR[0];
			posY = pos.VR[1];
			printf("VRx: %ld\n", posX);
			printf("VRy: %ld\n", posY);
		}


		/*
		 *        Input               Output
			 IN1 IN2 PWM STBY   OUT1 OUT2   Mode
			 H   H   H/L H      L    L 		Short brake
			 L   H   H   H      L    H 		CCW
			 L   H   L   H      L    L 		Short brake
			 H   L   H   H      H    L 		CW
			 H   L   L   H      L    L 		Short brake
			 L   L   H   H       OFF		Stop
			 	 	 	 	(High impedance)
			 H/L H/L H/L L       OFF		Standby
			 	 	 	 	(High impedance)
		 */

		if(posX > 3000)
		{
			//turn_right
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
		}
		else if(posX < 1000)
		{
			//turn_left
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
		}
		else
		{
			//no_turn
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
		}

		if(posY > 3000)
		{
			//forward
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
		}
		else if(posY < 1000)
		{
			//backward
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
		}
		else
		{
			//no_move
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
		}
	}
}
