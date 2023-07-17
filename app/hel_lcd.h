/**
 * @file hel_lcd.h
 * @brief This file initialices the functions of the LCD screen and the SPI module.
*/

#ifndef HEL_LCD_H
#define HEL_LCD_H

#include "app_bsp.h"

/**
  * @brief  Structure that contains the variables that will be used by the LCD and SPI module.
  */
typedef struct {
    SPI_HandleTypeDef   *SPIHandler;  /*!< Address of the SPI handler for LCD screen.*/
    GPIO_TypeDef        *RSTPort;     /*!< Port where the pin connected to the LCD RST is located.*/
    uint32_t            RSTPin;       /*!< Pin connected to the LCD reset.*/
    GPIO_TypeDef        *RSPort;      /*!< Port where the pin connected to the LCD RS is located.*/
    uint32_t            RSPin;        /*!< Pin connected to the LCD RS.*/
    GPIO_TypeDef        *CSPort;      /*!< Port where the pin connected to the LCD CS is located.*/
    uint32_t            CSPin;        /*!< Pin connected to the LCD CS.*/
    GPIO_TypeDef        *BKLPort;     /*!< Port where the pin connected to the LCD BKL is located.*/
    uint32_t            BKLPin;       /*!< Pin connected to the LCD BKL.*/
} LCD_HandleTypeDef;

/**
 * @brief Function to initialice the LCD.
*/

uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd );

/**
 * @brief Function to transmit the commands to LCD.
*/

uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd );

/**
 * @brief Function to transmit values to LCD.
*/

uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data );

/**
 * @brief Function to transmit strings to LCD.
*/

uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str );

/**
 * @brief Function that specifies the position where the LCD will print values.
*/

uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col);

/**
 * @brief Function that modifies the backlight level.
*/

void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state );

/**
 * @brief Function that modifies the contrast level.
*/

uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast );

#endif