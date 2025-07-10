#include "I2C_2.h"
// I2C handle
static I2C_HandleTypeDef *lcd_i2c;

// PCF8574 backlight control
#define LCD_BACKLIGHT 0x08 // P3 = 1 for backlight on
#define LCD_NO_BACKLIGHT 0x00 // P3 = 0 for backlight off

// LCD control bits
#define LCD_EN 0x04 // Enable bit (P2)
#define LCD_RS 0x01 // Register Select (P0)

// Send 4-bit data to PCF8574
static void LCD_Write4Bits(uint8_t data, uint8_t mode) {
  uint8_t i2c_data = (data & 0xF0) | mode | LCD_BACKLIGHT; // High nibble
  HAL_I2C_Master_Transmit(lcd_i2c, LCD_I2C_ADDRESS, &i2c_data, 1, HAL_MAX_DELAY);
  i2c_data |= LCD_EN; // Enable high
  HAL_I2C_Master_Transmit(lcd_i2c, LCD_I2C_ADDRESS, &i2c_data, 1, HAL_MAX_DELAY);
  HAL_Delay(1); // Pulse duration
  i2c_data &= ~LCD_EN; // Enable low
  HAL_I2C_Master_Transmit(lcd_i2c, LCD_I2C_ADDRESS, &i2c_data, 1, HAL_MAX_DELAY);
}

// Send full byte (4-bit mode: high nibble, then low nibble)
static void LCD_WriteByte(uint8_t data, uint8_t mode) {
  LCD_Write4Bits(data & 0xF0, mode); // High nibble
  LCD_Write4Bits((data << 4) & 0xF0, mode); // Low nibble
}


    /*
    * @brief This function initializes the LCD I2C half Byte communication on a provided I2C handler
    * @param &hadc*
    */
void LCD_Init(I2C_HandleTypeDef *hi2c) {
  lcd_i2c = hi2c;
  HAL_Delay(50); // Wait for LCD power-up

  // Initialization sequence (per HD44780 datasheet)
  LCD_Write4Bits(0x30, 0); // Function set (8-bit mode initially)
  HAL_Delay(5);
  LCD_Write4Bits(0x30, 0);
  HAL_Delay(1);
  LCD_Write4Bits(0x30, 0);
  HAL_Delay(1);
  LCD_Write4Bits(0x20, 0); // Switch to 4-bit mode
  HAL_Delay(1);

  // Now in 4-bit mode
  LCD_SendCommand(0x28); // Function set: 4-bit, 2 lines, 5x8 font
  LCD_SendCommand(0x08); // Display off
  LCD_Clear();           // Clear display
  LCD_SendCommand(0x06); // Entry mode: increment cursor, no shift
  LCD_SendCommand(0x0C); // Display on, cursor off, no blink
}

// Send command to LCD
void LCD_SendCommand(uint8_t cmd) {
  LCD_WriteByte(cmd, 0); // RS = 0 for command
}

// Send data (character) to LCD
void LCD_SendData(uint8_t data) {
  LCD_WriteByte(data, LCD_RS); // RS = 1 for data
}

// Send string to LCD
void LCD_SendString(char *str) {
  while (*str) {
    LCD_SendData(*str++);
  }
}

// Set cursor position (0-based: col 0–19, row 0–3)
void LCD_SetCursor(uint8_t col, uint8_t row) {
  uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54}; // 20x4 LCD DDRAM offsets
  if (row > 3) row = 0;
  LCD_SendCommand(0x80 | (col + row_offsets[row]));
}

// Clear display
void LCD_Clear(void) {
  LCD_SendCommand(0x01);
  HAL_Delay(2); // Clear takes ~2ms
}

void LCD_SendFloat(int valueInt,int valueDec) 
{
    char buffer[16]; // Buffer to hold the formatted string (adjust size as needed)
    
    // Format the float into a string with specified precision
    //snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
    snprintf(buffer, sizeof(buffer), "%.d.%d", valueInt, valueDec);
    LCD_SendString(buffer); // Send the string to the LCD
  }
  void LCD_SendInt(int value) {
    char buffer[12]; // Enough for -2147483648 and null terminator
    snprintf(buffer, sizeof(buffer), "%d", value);
    LCD_SendString(buffer);
}
