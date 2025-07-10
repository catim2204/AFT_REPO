/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <user.h>
#include <stdbool.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//  #pragma pack(push,1)
// typedef struct Received_Data_STR 
// {
//         uint8_t Start;
//         uint8_t S_NUM;
//         uint8_t Blade_Count;
//         uint8_t Current_Limit_Steady;
//         uint8_t Throttle;
//         bool ARM_Check;
//         bool MANUAL_Check;
//         bool Start_Test;
//         bool UI_Mode;
//         bool Save_To_Controller;
//         bool Calibration;
//         uint8_t Load_Cell_Thrust;
//         uint8_t Load_Cell_Torque;
//         uint8_t Stop;
// } Received_Data_STR;
// #pragma pack(pop)
// extern  Received_Data_STR Received_Data;
// #pragma pack(push,1)
// typedef struct Sent_Data_STR
// {       
//         int16_t  Voltage_I;
//         uint16_t  Voltage_F;
//         int16_t  Current_I;
//         uint16_t  Current_F;
//         uint16_t  RPM_I;
//         uint16_t  RPM_F;
//         int16_t  Thrust_I;
//         uint16_t  Thrust_F;
//         int16_t  Torque_I;
//         uint16_t  Torque_F;
//         uint8_t   Throttle;
//         uint16_t  PWM_Time;
//         uint8_t   Error_Code;
// }Sent_Data_STR;
// #pragma pack(pop)
// extern  Sent_Data_STR Sent_Data;
// extern volatile bool Com_Method;
// extern volatile bool Data_Received_Flag;

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

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
