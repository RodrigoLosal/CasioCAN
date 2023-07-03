#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

extern void initialise_monitor_handles( void );
static void LED_Init( void );
static void Heart_Beat( void );

int main( void )
{
    HAL_Init();
    Serial_Init();
    Clock_Init();
    LED_Init();
    initialise_monitor_handles();

    while( 1 )
    {
        Serial_Task();
        Clock_Task();
        Heart_Beat();
    }
}

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

static void Heart_Beat( void ) {
    uint32_t TickStartHeart = 0;

    if( (HAL_GetTick() - TickStartHeart) >= 300 ) {
        TickStartHeart = HAL_GetTick();
        HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 );
    }
}