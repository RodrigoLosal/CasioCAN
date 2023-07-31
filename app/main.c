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
#include "scheduler.h"

/**
 * @defgroup Tasks & tick time (ms) for the functioning of the scheduler.
 @{*/
#define TASKS_N     5       /*!< Specifies the number of tasks.*/
#define TICK_VAL    10      /*!< Value of the tick.*/
/**@} */

static void LED_Init( void );
static void Dog_Init( void );
static void Heart_Beat( void );
static void Pet_The_Dog( void );

/**
  * @brief   Structure that will contain the values to initialice the Watchdog Timer.
  */
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
    static Task_TypeDef tasks[ TASKS_N ];
    static Scheduler_HandleTypeDef Sche;

    /*The function is used and its result is verified.*/
    Status = HAL_Init();
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, HAL_RET_ERROR );

    /*Initialice the scheduler with the number of tasks and the value of the time tick.*/
    Sche.tick = TICK_VAL;
    Sche.tasks = TASKS_N;
    Sche.taskPtr = tasks;
    HIL_SCHEDULER_Init( &Sche );

    /*Register tasks with thier corresponding init fucntions and their periodicyt*/
    ( void )HIL_SCHEDULER_RegisterTask( &Sche, LED_Init, Heart_Beat, 300 );
    ( void )HIL_SCHEDULER_RegisterTask( &Sche, Serial_Init, Serial_Task, 10 );
    ( void )HIL_SCHEDULER_RegisterTask( &Sche, Clock_Init, Clock_Task, 50 );
    ( void )HIL_SCHEDULER_RegisterTask( &Sche, Display_Init, Display_Task, 100 );
    ( void )HIL_SCHEDULER_RegisterTask( &Sche, Dog_Init, Pet_The_Dog, 75 );
        
    /*Run the scheduler in a infinite loop*/
    HIL_SCHEDULER_Start( &Sche );
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
 * Clock Cycle Time: 1 / ( ( APB = 32MHz ) / 4096 ) / 128 = 2048 us = 2.048 ms
 * Timout:
 * tWWDG = ( 1 / ( 32MHz / 4096 ) / 16 ) * ( 127 + 1 ) = 0.2622 s = 262.2 ms
 * With Window value of 94:
 * tWWDG = ( 1 / ( 32MHz / 4096 ) / 16 ) * ( 94 + 1 ) = 0.1946 s = 194.6 ms
 * Difference:
 * 262.2 ms  - 194.6 ms = 67.6 ms --> Refresh > 67.6 ms ∴ 75 ms proposed.
 *
 * @param   WDGHandler[out] Structure-type variable to save the configuration of the Watchdog registers.
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

/**
 * @brief WWDG interrupt.
 * @param[in] hwddg
 */
/*cppcheck-suppress misra-c2012-8.4 ; Function provided by Hal.*/
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwddg)
{
    (void)hwddg;
    assert_error(0u, WWDG_FUNC_ERROR); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
}