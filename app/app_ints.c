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

/*cppcheck-suppress misra-c2012-8.4 ; Function provide by Hal*/
/**
 * @brief Declare CAN interrupt
 * Declare WWDG interrupt service rutine as it is declare in startup_stm32g0b1xx.s file
 */
void WWDG_IRQHandler(void)
{
    HAL_WWDG_IRQHandler(&WDGHandler);
}

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
    assert_error(0u, HARDFAULT_FUNC_ERROR); /*cppcheck-suppress misra-c2012-11.8 ; Function can not be modified.*/
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

/*cppcheck-suppress misra-c2012-8.4 ; Function provided by the HAL library.*/
/**
 * @brief CAN interrupt
 * Declare CAN interrupt service rutine as it is declare in startup_stm32g0b1xx.s file
 * @param[in] hfdcan
 */
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
    (void)hfdcan;
    assert_error(0u, CAN_FUNC_ERROR); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
}

/*cppcheck-suppress misra-c2012-8.4 ; Function provided by the HAL library.*/
/**
 * @brief SPI interrupt
 * @param[in] hspi
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
    assert_error(0u, SPI_FUNC_ERROR); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
}

/*cppcheck-suppress misra-c2012-8.4 ; Function provided by the HAL library.*/
/**
 * @brief Flash interrupt
 * @param[in] ReturnValue
 */
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
    (void)ReturnValue;
    assert_error(0u, FLASH_FUNC_ERROR); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
}