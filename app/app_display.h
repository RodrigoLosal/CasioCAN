/**
 * @file app_display.h
 * @brief This file initialices the LCD screen and the SPI module with state machine of the LCD tasks.
*/

#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "app_bsp.h"

/**
 * @brief Function to initialice the LCD.
*/

void Display_Init( void );

/**
 * @brief Function of the state machine of the LCD tasks.
 * state machine in charge of messages processing from the clock task and display the time and date.
*/

void Display_Task( void );

#endif