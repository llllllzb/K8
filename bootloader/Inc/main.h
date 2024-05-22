/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal.h"

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
#define POWEREN4G_Pin GPIO_PIN_1
#define POWEREN4G_GPIO_Port GPIOA
#define CHARGE_Pin GPIO_PIN_4
#define CHARGE_GPIO_Port GPIOA
#define WAKEUP_4G_Pin GPIO_PIN_0
#define WAKEUP_4G_GPIO_Port GPIOB
#define PWRKEY_4G_Pin GPIO_PIN_12
#define PWRKEY_4G_GPIO_Port GPIOB
#define RSTKEY_4G_Pin GPIO_PIN_14
#define RSTKEY_4G_GPIO_Port GPIOB
#define GNSSLNA_Pin GPIO_PIN_15
#define GNSSLNA_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_15
#define LED_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

//4G电源
#define POWEROFMODULEON				HAL_GPIO_WritePin(POWEREN4G_GPIO_Port,POWEREN4G_Pin, GPIO_PIN_SET)
#define POWEROFMODULEOFF			HAL_GPIO_WritePin(POWEREN4G_GPIO_Port,POWEREN4G_Pin, GPIO_PIN_RESET)
//4G开机键
#define POWKEYOFMODULEON		    HAL_GPIO_WritePin(PWRKEY_4G_GPIO_Port,PWRKEY_4G_Pin, GPIO_PIN_SET)
#define POWKEYOFMODULEOFF		    HAL_GPIO_WritePin(PWRKEY_4G_GPIO_Port,PWRKEY_4G_Pin, GPIO_PIN_RESET)
//4G复位键
#define RSTKEYOFMODULEON		    HAL_GPIO_WritePin(RSTKEY_4G_GPIO_Port,RSTKEY_4G_Pin, GPIO_PIN_SET)
#define RSTKEYOFMODULEOFF		    HAL_GPIO_WritePin(RSTKEY_4G_GPIO_Port,RSTKEY_4G_Pin, GPIO_PIN_RESET)

#define LED_TOGGLE       HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin)  

#define CHARGEDET      HAL_GPIO_ReadPin(CHARGE_GPIO_Port,CHARGE_Pin)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
