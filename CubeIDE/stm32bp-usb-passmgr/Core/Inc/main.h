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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Blue_Pin GPIO_PIN_13
#define LED_Blue_GPIO_Port GPIOC
#define KEY_User_Pin GPIO_PIN_0
#define KEY_User_GPIO_Port GPIOA
#define KEY_User_EXTI_IRQn EXTI0_IRQn
#define Button_A_Pin GPIO_PIN_7
#define Button_A_GPIO_Port GPIOA
#define Button_A_EXTI_IRQn EXTI9_5_IRQn
#define Button_B_Pin GPIO_PIN_5
#define Button_B_GPIO_Port GPIOB
#define Button_B_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */
void printLCD(char*);
void setCapslockState(uint8_t);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
