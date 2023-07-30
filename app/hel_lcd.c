/**
 * @file hel_lcd.c
 * @brief This file initialices the LCD operation and runs its functions.
*/

#include "hel_lcd.h"
#include "app_bsp.h"

/**
 * @defgroup Backlight states
 @{*/
#define LCD_ON 1        /*!< State 1 baklight on*/
#define LCD_OFF 0       /*!< State 0 baklight off*/
#define LCD_TOGGLE 2    /*!< State 2 toggle state*/
/**@} */

uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd ){
    assert_error( (hlcd->CSPort == GPIOD), LCD_PAR_ERROR );      /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->RSTPort == GPIOD), LCD_PAR_ERROR );     /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->RSPort == GPIOD), LCD_PAR_ERROR );      /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->BKLPort == GPIOB), LCD_PAR_ERROR );     /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->CSPin == GPIO_PIN_3), LCD_PAR_ERROR );  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->RSTPin == GPIO_PIN_2), LCD_PAR_ERROR ); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/ 
    assert_error( (hlcd->RSPin == GPIO_PIN_4), LCD_PAR_ERROR );  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->BKLPin == GPIO_PIN_4), LCD_PAR_ERROR ); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( (hlcd->SPIHandler != NULL), LCD_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

    uint8_t resulInit = 0;
    HEL_LCD_MspInit( hlcd );

    //CS = 1;
    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, SET);
    //RST = 0;
    HAL_GPIO_WritePin(hlcd->RSTPort, hlcd->RSTPin, RESET);
    HAL_Delay( 2 );
    //RST = 1;
    HAL_GPIO_WritePin(hlcd->RSTPort, hlcd->RSTPin, SET);
    HAL_Delay( 20 );
    resulInit = HEL_LCD_Command( hlcd, 0x30 ); /*Wakeup.*/
    HAL_Delay( 2 );
    resulInit = HEL_LCD_Command( hlcd, 0x30 ); /*Wakeup.*/
    resulInit = HEL_LCD_Command( hlcd, 0x30 ); /*Wakeup.*/
    resulInit = HEL_LCD_Command( hlcd, 0x39 ); /*Function set.*/
    resulInit = HEL_LCD_Command( hlcd, 0x14 ); /*Internal osc frequency.*/
    resulInit = HEL_LCD_Command( hlcd, 0x56 ); /*Power controll.*/
    resulInit = HEL_LCD_Command( hlcd, 0x6d ); /*Follower control.*/
    HAL_Delay( 200 );
    resulInit = HEL_LCD_Command( hlcd, 0x70 ); /*Constrast.*/
    resulInit = HEL_LCD_Command( hlcd, 0x0C ); /*Display on.*/
    resulInit = HEL_LCD_Command( hlcd, 0x06 ); /*Entry mode.*/
    resulInit = HEL_LCD_Command( hlcd, 0x01 ); /*Clear screen.*/
    HAL_Delay( 1 );
    return resulInit;
}

uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd ){
    uint8_t Result;

    HAL_GPIO_WritePin( hlcd->RSPort, hlcd->RSPin, RESET ); /*Command mode.*/
    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, RESET ); /*Chip select on.*/
    Result = HAL_SPI_Transmit( hlcd->SPIHandler, &cmd, 1, 5000 );
    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, SET ); /*Chip select off.*/
    return Result;
}

/*cppcheck-suppress misra-c2012-8.7 ; Function provide by Hal*/
uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data ){
    uint8_t Result;

    HAL_GPIO_WritePin( hlcd->RSPort, hlcd->RSPin, SET ); /*Data mode*/
    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, RESET ); /*Chip select on*/
    Result = HAL_SPI_Transmit( hlcd->SPIHandler, &data, 1, 5000 );
    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, SET ); /*Chip select off*/
    return Result;
}

uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str ){
    uint8_t Result = 0;

    HAL_GPIO_WritePin( hlcd->RSPort, hlcd->RSPin, SET ); /*Data mode.*/ 
    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, RESET ); /*Chip select on.*/

    for( uint8_t i=0; str[i] != '\0'; i++ ){
        Result= HEL_LCD_Data( hlcd, str[i] );
    }

    HAL_GPIO_WritePin( hlcd->CSPort, hlcd->CSPin, SET ); /*Chip select off.*/
    return Result;
}

uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col ){
    uint8_t Result;

    if( row == ( uint8_t ) 0 ) { /*First row.*/
        Result = HEL_LCD_Command( hlcd, 0x80 );
        if((col >= ( uint8_t ) 0 ) && ( col <= ( uint8_t ) 15 ) ) { 
           Result = HEL_LCD_Command(hlcd, col + (uint8_t)0x80); 
        }
    }else if( row == ( uint8_t ) 1 ) { /*Second row.*/
        Result = HEL_LCD_Command( hlcd, 0xC0 );
        if((col >= ( uint8_t ) 0 ) && ( col <= ( uint8_t ) 15 ) ) {
           Result = HEL_LCD_Command( hlcd, col  + ( uint8_t ) 0xC0 ); 
        }
    }else{
        Result = 1;
    }

    return Result;
}

void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state ){
    if( state == (uint8_t)LCD_ON ){
        HAL_GPIO_WritePin( hlcd->BKLPort, hlcd->BKLPin, SET );
    } else if( state  == ( uint8_t ) LCD_OFF ) {
        HAL_GPIO_WritePin( hlcd->BKLPort, hlcd->BKLPin, RESET );
    } else if(state  == ( uint8_t ) LCD_TOGGLE ) {
        HAL_GPIO_TogglePin( hlcd->BKLPort, hlcd->BKLPin );
    }else{
    }
}

uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast ){
    uint8_t Result = 0;
    if( ( contrast >= ( uint8_t ) 0 ) && ( contrast <= ( uint8_t) 15 ) ) { 
        Result = HEL_LCD_Command( hlcd, contrast + ( uint8_t ) 0x70 ); 
    }else{
    }
    return Result;
}