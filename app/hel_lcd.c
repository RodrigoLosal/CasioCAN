/**
 * @file hel_lcd.c
 * @brief This file initialices the LCD operation and runs its functions.
*/

#include "hel_lcd.h"

/**
 * @defgroup Backlight states.
 @{*/
#define LCD_OFF 0       /*!< State 0: backlight off.*/
#define LCD_ON 1        /*!< State 1: backlight on.*/
#define LCD_TOGGLE 2    /*!< State 2: toggle state.*/
/**@} */

uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd ) {
    uint8_t InitResult = 0;

    HEL_LCD_MspInit( hlcd );

    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, SET); /*CS = 1 -> Chip select OFF*/
    HAL_GPIO_WritePin(hlcd->RSTPort, hlcd->RSTPin, RESET); /*RST = 0*/

    HAL_Delay( 2 );

    HAL_GPIO_WritePin(hlcd->RSTPort, hlcd->RSTPin, SET); /*RST = 1*/

    HAL_Delay( 20 );

    InitResult = HEL_LCD_Command( hlcd, 0x30 ); /*Wakeup*/

    HAL_Delay( 2 );

    InitResult = HEL_LCD_Command( hlcd, 0x30 ); /*Wakeup*/
    InitResult = HEL_LCD_Command( hlcd, 0x30 ); /*Wakeup*/
    InitResult = HEL_LCD_Command( hlcd, 0x39 ); /*Function set*/
    InitResult = HEL_LCD_Command( hlcd, 0x14 ); /*Internal osc frequency*/
    InitResult = HEL_LCD_Command( hlcd, 0x56 ); /*Power control*/
    InitResult = HEL_LCD_Command( hlcd, 0x6d ); /*Follower control*/

    HAL_Delay( 200 );

    InitResult = HEL_LCD_Command( hlcd, 0x70 ); /*Constrast*/
    InitResult = HEL_LCD_Command( hlcd, 0x0C ); /*Display on*/
    InitResult = HEL_LCD_Command( hlcd, 0x06 ); /*Entry mode*/
    InitResult = HEL_LCD_Command( hlcd, 0x01 ); /*Clear screen*/

    HAL_Delay( 1 );

    return InitResult;
}

uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd ){
    uint8_t OpResult = 0;

    HAL_GPIO_WritePin(hlcd->RSPort, hlcd->RSPin, RESET); /*RS = 0 -> Command mode*/
    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, RESET); /*CS = 0 -> Chip select ON*/ 
    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, SET); /*CS = 1 -> Chip select OFF*/
    return OpResult;
}

/*cppcheck-suppress misra-c2012-8.7 ; Function provide by Hal*/
uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data ){
    uint8_t OpResult;

    HAL_GPIO_WritePin(hlcd->RSPort, hlcd->RSPin, SET); /*RS = 1 -> Data mode*/
    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, RESET); /*CS = 0 -> Chip select ON*/
    OpResult = HAL_SPI_Transmit( hlcd->SPIHandler, &data, 1, 5000 );
    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, SET); /*CS = 1 -> Chip select OFF*/
    return OpResult;
}

uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str ){
    uint8_t OpResult = 0;
    uint8_t i=0;

    HAL_GPIO_WritePin(hlcd->RSPort, hlcd->RSPin, SET); /*RS = 1 -> Data mode*/ 
    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, RESET); /*CS = 0 -> Chip select ON*/

    while( str[ i ] != '\0') {
        OpResult= HEL_LCD_Data(hlcd, str[i]);
        i++;
    }

    HAL_GPIO_WritePin(hlcd->CSPort, hlcd->CSPin, SET); /*CS = 1 -> Chip select OFF*/
    return OpResult;
}

uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col ){
    uint8_t OpResult;

    if(row == (uint8_t)0){ /*First row*/
        OpResult = HEL_LCD_Command(hlcd, 0x80);
        if((col >= (uint8_t)0) && (col <= (uint8_t)15)){ 
           OpResult = HEL_LCD_Command(hlcd, col + (uint8_t)0x80); 
        }
    }else if(row == (uint8_t)1){ /*Second row*/
        OpResult = HEL_LCD_Command(hlcd, 0xC0);
        if((col >= (uint8_t)0) && (col <= (uint8_t)15)){
           OpResult = HEL_LCD_Command(hlcd, col  + (uint8_t)0xC0); 
        }
    }else{
        OpResult = 1;
    }

    return OpResult;
}

void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state ){
    if(state == (uint8_t)LCD_ON) {
        HAL_GPIO_WritePin( hlcd->BKLPort, hlcd->BKLPin, SET );
    }
    else if(state  == (uint8_t)LCD_OFF) {
        HAL_GPIO_WritePin( hlcd->BKLPort, hlcd->BKLPin, RESET );
    }
    else if(state  == (uint8_t)LCD_TOGGLE) {
        HAL_GPIO_TogglePin(hlcd->BKLPort, hlcd->BKLPin);
    }
    else{
    }
}

uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast ){
    uint8_t OpResult = 0;
    if((contrast >= (uint8_t)0) && (contrast <= (uint8_t)15)){ 
        OpResult = HEL_LCD_Command(hlcd, contrast + (uint8_t)0x70); 
    }
    else{
    }
    return OpResult;
}