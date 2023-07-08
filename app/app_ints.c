/**------------------------------------------------------------------------------------------------
 * File with the interrupt functions of the microcontrollers, check the file startup_stm32g0b1.S
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"

extern void NMI_Handler( void );
extern void HardFault_Handler( void );
extern void SVC_Handler( void );
extern void PendSV_Handler( void );
extern void SysTick_Handler( void );
extern void TIM16_FDCAN_IT0_IRQHandler( void );

/**------------------------------------------------------------------------------------------------
Brief.- Program entry point.
-------------------------------------------------------------------------------------------------*/
void NMI_Handler( void )
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Program entry point.
-------------------------------------------------------------------------------------------------*/
void HardFault_Handler( void )
{
    assert_param( 0u );
}

/**------------------------------------------------------------------------------------------------
Brief.- Program entry point.
-------------------------------------------------------------------------------------------------*/
void SVC_Handler( void )
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Program entry point.
-------------------------------------------------------------------------------------------------*/
void PendSV_Handler( void )
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Program entry point.
-------------------------------------------------------------------------------------------------*/
void SysTick_Handler( void )
{
    HAL_IncTick( );
}

/*Declare CAN interrupt service rutine as it is declare in startup_stm32g0b1xx.s file*/    
void TIM16_FDCAN_IT0_IRQHandler( void )
{
    /*HAL library functions that attend interrupt on CAN*/
    HAL_FDCAN_IRQHandler( &CANHandler );
}