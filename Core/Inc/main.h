/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define SLEEP_TRACK_PIN_Pin GPIO_PIN_1
#define SLEEP_TRACK_PIN_GPIO_Port GPIOA
#define VBAT_LEVEL_Pin GPIO_PIN_5
#define VBAT_LEVEL_GPIO_Port GPIOA
#define LED_RED_Pin GPIO_PIN_6
#define LED_RED_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOA
#define LED_BLUE_Pin GPIO_PIN_0
#define LED_BLUE_GPIO_Port GPIOB
#define LED_YELLOW_Pin GPIO_PIN_1
#define LED_YELLOW_GPIO_Port GPIOB
#define KEY_MENU_Pin GPIO_PIN_10
#define KEY_MENU_GPIO_Port GPIOB
#define KEY_MENU_EXTI_IRQn EXTI15_10_IRQn
#define KEY_YELLOW_Pin GPIO_PIN_12
#define KEY_YELLOW_GPIO_Port GPIOB
#define KEY_YELLOW_EXTI_IRQn EXTI15_10_IRQn
#define KEY_BLUE_Pin GPIO_PIN_13
#define KEY_BLUE_GPIO_Port GPIOB
#define KEY_BLUE_EXTI_IRQn EXTI15_10_IRQn
#define KEY_GREEN_Pin GPIO_PIN_14
#define KEY_GREEN_GPIO_Port GPIOB
#define KEY_GREEN_EXTI_IRQn EXTI15_10_IRQn
#define KEY_RED_Pin GPIO_PIN_15
#define KEY_RED_GPIO_Port GPIOB
#define KEY_RED_EXTI_IRQn EXTI15_10_IRQn
#define AMP_MUTE_Pin GPIO_PIN_4
#define AMP_MUTE_GPIO_Port GPIOB
#define AMP_SHUTDOWN_Pin GPIO_PIN_5
#define AMP_SHUTDOWN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
