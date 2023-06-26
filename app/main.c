#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

extern void initialise_monitor_handles( void );

int main( void )
{
    HAL_Init();
    Serial_Init();
    Clock_Init();
    initialise_monitor_handles();

    while( 1 )
    {
        Serial_Task();
        Clock_Task();
    }
}