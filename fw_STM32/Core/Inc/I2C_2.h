#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "stm32f1xx_hal.h" // Adjust for your STM32 series (e.g., stm32f4xx_hal.h)
#include <stdio.h>
#define LCD_I2C_ADDRESS 0b01001110 // Adjust based on your I2C address (e.g., 0x4E or 0x7E)

void LCD_Init(I2C_HandleTypeDef *hi2c);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SendString(char *str);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_Clear(void);
void LCD_SendFloat(int valInt,int valDec);
void LCD_SendInt(int value); 
#endif