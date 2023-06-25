/**------------------------------------------------------------------------------------------------
 * Archivo con la funciones de las incilaizaciones auxiliares de la libreria
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"

void HAL_MspInit( void )
{
    
}

/*This function is called in HAL_FDCAN_Init*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
    GPIO_InitTypeDef GpioCanStruct;

    /* Habilitamos los relojes de los perifericos GPIO y CAN */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /* configuramos pin 0(rx) y pin 1(tx) en modo alterno para FDCAN1 */
    GpioCanStruct.Mode = GPIO_MODE_AF_PP;
    GpioCanStruct.Alternate = GPIO_AF3_FDCAN1;
    GpioCanStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GpioCanStruct.Pull = GPIO_NOPULL;
    GpioCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOD, &GpioCanStruct );

    /*Enable vector interrupt to attend CAN IRQs */
    HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
}