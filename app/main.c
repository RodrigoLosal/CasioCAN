/**
 * @file    main.c
 * @brief   **File that is used as the authority of the program functioning.**
 *
 * This file initializes and executes the functions required for the operation of the program, 
 * these can come from auxiliary files or are defined locally. The sequence starts with the function 
 * that controls the serial communication used to set the clock values, then uses a function 
 * that controls the data display, then a function that toggles an LED, and finally 
 * a function to refresh the watchdog timer.
 */

#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
#include "app_display.h"

static void LED_Init( void );
static void Dog_Init( void );
static void Heart_Beat( void );
static void Pet_The_Dog( void );

/**
  * @brief   Structure that will contain the values to initialice the Watchdog Timer.
  */

extern WWDG_HandleTypeDef WDGHandler;
WWDG_HandleTypeDef WDGHandler = {0};

/**
 * @brief  Global variable for the Heartbeat function time counter.
 */

extern uint32_t TickStartHeart;
uint32_t TickStartHeart = 0;

/**
 * @brief  Global variable for the Watchdog function time counter.
 */

extern uint32_t TickStartWDog;
uint32_t TickStartWDog = 0;

/**
 * @brief   **Main function.**
 *
 * Initialices and executes all of the functions of the application.
 *
 */

int main( void )
{
    HAL_StatusTypeDef Status;

    /*The function is used and its result is verified.*/
    Status = HAL_Init();
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, HAL_RET_ERROR );

    Serial_Init();
    Clock_Init();
    Display_Init();
    LED_Init();
    //Dog_Init();

    while( 1 )
    {
        Serial_Task();
        Clock_Task();
        Display_Task();
        Heart_Beat();
        //Pet_The_Dog();
    }
}

/**
 * @brief   **Function to set the GPIO registers required to toggle an LED light.**
 *
 * Only the Port C - Pin 0 will be used, and it's initial state will be stablished as RESET.
 * 
 */

static void LED_Init( void ) {
    GPIO_InitTypeDef  GPIO_InitStruct;

    __HAL_RCC_GPIOC_CLK_ENABLE( );

    GPIO_InitStruct.Pin   = GPIO_PIN_0;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_0, RESET );
}

/**
 * @brief   **Function to set the parameters of the Watchdog Timer.**
 *
 * The Watchdog will operate in the Windowed configuration, with the following calculations to support
 * the chosen parameters:
 * 
 * Clock Cycle Time: 1 / ( ( APB = 32MHz ) / 4096 ) / 128 = 16384 us = 16.384 ms
 * Timout:
 * tWWDG = ( 1 / ( 32MHz / 4096 ) / 128 ) * ( 127 + 1 ) = 2.0972 s
 * With Window value of 94:
 * tWWDG = ( 1 / ( 32MHz / 4096 ) / 128 ) * ( 97 + 1 ) = 1.8186 s
 * Difference:
 * 524.29 ms  - 401.41 ms = 278.6 ms
 *
 * @param   WDGHandler[out] Struct-type variable to save the configuration of the Watchdog registers.
 *
 */
static void Dog_Init( void ) {
    HAL_StatusTypeDef Status;

    __HAL_RCC_WWDG_CLK_ENABLE();

    WDGHandler.Instance = WWDG;
    WDGHandler.Init.Prescaler = WWDG_PRESCALER_16;
    WDGHandler.Init.Window = 94;
    WDGHandler.Init.Counter = 127;
    WDGHandler.Init.EWIMode = WWDG_EWI_ENABLE;

    /*The function is used and its result is verified.*/
    Status = HAL_WWDG_Init( &WDGHandler );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, WWDG_RET_ERROR );
    HAL_NVIC_SetPriority( WWDG_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ( WWDG_IRQn );
}

/**
 * @brief   **Function to toggle an LED light.**
 *
 * The LED toggling will be every 300ms.
 *
 */
static void Heart_Beat( void ) {
    if( (HAL_GetTick() - TickStartHeart) >= 300 ) {
        TickStartHeart = HAL_GetTick();
        HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 );
    }
}

/**
 * @brief   **Function to refresh the Watchdog timer.**
 *
 * According to the calculations, the window to refresh happens between X ms and X ms, a middle position
 * between these two numbers was chosen as the sweet-spot.
 *
 */
static void Pet_The_Dog( void ) {
    HAL_StatusTypeDef Status;
    
    if( (HAL_GetTick() - TickStartWDog) >= 75u ) {
        TickStartWDog = HAL_GetTick();

        /*The function is used and its result is verified.*/
        Status = HAL_WWDG_Refresh( &WDGHandler );
        /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
        assert_error( Status == HAL_OK, WWDG_RET_ERROR );
    }
}


/**
 * @brief   **Safe State Function.**
 *
 * All the internal clocks will be desabled and the uC will wait for the user to make a hard reset, 
 * the LEDs in Port C will display the error code in binary format.
 *
 */

void Safe_State( uint8_t *file, uint32_t line, uint8_t error ) {
    (void) file;
    (void) line;

    /*Disable all maskable interrupts.*/
    __disable_irq();

    /*Disable FDCAN module.*/
    HAL_GPIO_DeInit( GPIOD, GPIO_PIN_0 | GPIO_PIN_1 );
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /*Disable SPI module.*/
    HAL_GPIO_DeInit( GPIOD, GPIO_PIN_6 | GPIO_PIN_8 );
    __SPI1_CLK_DISABLE();

    /*Disable Heartbeat LED pin.*/
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_0 );

    /*Disable the rest of the clocks*/
    __HAL_RCC_SYSCFG_CLK_DISABLE();
    __HAL_RCC_PWR_CLK_DISABLE();

    /*Disable all timers.*/
    __HAL_RCC_RTC_DISABLE();

    /**/
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = 0x00FF;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    HAL_GPIO_WritePin( GPIOC, error, GPIO_PIN_SET );
  
    while( 1 ) {
        /*Waiting for the user to press the reset button*/
    }
}

/*cppcheck-suppress misra-c2012-8.4 ; Function provided by Hal.*/
/**
 * @brief WWDG interrupt.
 * @param[in] hwddg
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwddg)
{
    (void)hwddg;
    assert_error(0u, WWDG_FUNC_ERROR); /*cppcheck-suppress misra-c2012-11.8 ; Function can not be modify*/
}