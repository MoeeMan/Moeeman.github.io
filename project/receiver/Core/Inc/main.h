/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include <stdlib.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern TaskHandle_t ultrasonic_task_handle;

extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern QueueHandle_t eventQueue;

extern UART_HandleTypeDef huart2;

typedef enum {
	ULTRASONIC = (uint8_t)0x1,
	JOYSTICK   = (uint8_t)0x2
}EVENT_TYPE;

typedef struct{
	EVENT_TYPE eventType;
	uint32_t data[5];	//either median or joystick reading
}QUEUE_DATA;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void receive_task(void *param);
void motor_task(void *param);
void ultrasonic_task(void *param);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BackMotor_IN1_Pin GPIO_PIN_0
#define BackMotor_IN1_GPIO_Port GPIOA
#define BackMotor_IN2_Pin GPIO_PIN_1
#define BackMotor_IN2_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define CE_Pin GPIO_PIN_3
#define CE_GPIO_Port GPIOA
#define CSN_Pin GPIO_PIN_4
#define CSN_GPIO_Port GPIOA
#define IRQ_Pin GPIO_PIN_0
#define IRQ_GPIO_Port GPIOB
#define Echo_Pin GPIO_PIN_1
#define Echo_GPIO_Port GPIOB
#define PWMA_Pin GPIO_PIN_8
#define PWMA_GPIO_Port GPIOA
#define PWMB_Pin GPIO_PIN_9
#define PWMB_GPIO_Port GPIOA
#define HeadLights_Pin GPIO_PIN_10
#define HeadLights_GPIO_Port GPIOA
#define FrontMotor_IN1_Pin GPIO_PIN_11
#define FrontMotor_IN1_GPIO_Port GPIOA
#define FrontMotor_IN2_Pin GPIO_PIN_12
#define FrontMotor_IN2_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB
#define Trig_Pin GPIO_PIN_5
#define Trig_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define DEBUG_UART 0

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
