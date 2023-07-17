/**
 * @file app_display.c
 * @brief Init the LCD and SPI withnstate machine
*/

#include "app_display.h"
#include "hel_lcd.h"
#include "app_bsp.h"

/**
 * @defgroup Display states
 @{*/
#define IDLE 1          /*!< State one of clock state machine.*/
#define TRANSMIT 2  /*!< State two of clock state machine.*/
/**@}*/

/**
 * @brief  Variable to indicate the state of clock state machine.
*/
extern uint8_t state_lcd;
uint8_t state_lcd = IDLE;

/**
 * @brief This is a struct variable to contain the members of LCD.
 */ 
extern LCD_HandleTypeDef  hLcd;
LCD_HandleTypeDef  hLcd = {0};
/**
 * @brief This is a struct variable to contain the members of SPI.
 */ 
extern SPI_HandleTypeDef SPI_Handler;     
SPI_HandleTypeDef SPI_Handler = {0}; 

static void DateString(char *string, unsigned char month, unsigned char day, unsigned short year, unsigned char weekday);
static void TimeString(char *string, unsigned char hours, unsigned char minutes, unsigned char seconds);

static char cadenaTime[9];
static char cadenaDate[16];
char *pCadena = cadenaTime;
char *pCadenaDate = cadenaDate;

void Display_Init( void ){
    /*Configuration of pins and port with the LCD struc*/
    hLcd.RSTPort = GPIOD;
    hLcd.RSPort = GPIOD;
    hLcd.CSPort = GPIOD;
    hLcd.BKLPort = GPIOB;
    hLcd.RSTPin = GPIO_PIN_2;
    hLcd.RSPin = GPIO_PIN_4;
    hLcd.CSPin = GPIO_PIN_3;
    hLcd.BKLPin = GPIO_PIN_4;

    hLcd.SPIHandler = &SPI_Handler;
         
    /*Configure the spi in master mode, full-duplex communication, 
    clock polarity high and phase on falling edge */
    SPI_Handler.Instance            = SPI1;
    SPI_Handler.Init.Mode           = SPI_MODE_MASTER;
    SPI_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; 
    SPI_Handler.Init.Direction      = SPI_DIRECTION_2LINES;
    SPI_Handler.Init.CLKPhase       = SPI_PHASE_2EDGE;
    SPI_Handler.Init.CLKPolarity    = SPI_POLARITY_HIGH;
    SPI_Handler.Init.DataSize       = SPI_DATASIZE_8BIT;
    SPI_Handler.Init.FirstBit       = SPI_FIRSTBIT_MSB;
    SPI_Handler.Init.NSS            = SPI_NSS_SOFT;
    SPI_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    SPI_Handler.Init.TIMode         = SPI_TIMODE_DISABLED;

    HAL_SPI_Init( &SPI_Handler );
    (void)HEL_LCD_Init(&hLcd);
}

void Display_Task( void ){
    switch(state_lcd){
        case IDLE:
            if(ClockMsg.msg == (uint8_t)1){
                ClockMsg.msg = 0;
                state_lcd = TRANSMIT;
            }
        break;

        case TRANSMIT:
            (void)HEL_LCD_SetCursor( &hLcd, 1, 3);
            TimeString( pCadena, ClockMsg.tm.tm_hour, ClockMsg.tm.tm_min, ClockMsg.tm.tm_sec );
            (void)HEL_LCD_String( &hLcd, pCadena);
            
            (void)HEL_LCD_SetCursor( &hLcd, 0, 1);
            DateString(pCadenaDate, ClockMsg.tm.tm_mon, ClockMsg.tm.tm_mday, ClockMsg.tm.tm_year, ClockMsg.tm.tm_wday);
            (void)HEL_LCD_String( &hLcd, pCadenaDate);
        break;

        default:
        break;
    }
}

static void DateString(char *string, unsigned char month, unsigned char day, unsigned short year, unsigned char weekday) {
    char dias[][4] = {"Lu", "Ma", "Mi", "Ju", "Vi", "Sa", "Do"};
    char mes[][4] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    
    strcat(string, mes[month-1]); 
    string[3] = ',';
    string[4] = (day / 10 + '0');
    string[5] = (day % 10 + '0');
    string[6] = ' ';
    string[7] = (year / 1000 + '0');
    string[8] = (year % 1000 /100 + '0');
    string[9] = (year % 1000 %100 /10 + '0');
    string[10] = (year % 1000 %100 %10 /1 + '0');
    string[11] = ' ';
    strcpy(&string[12], dias[weekday-1]);
    string[14] = '\0';      
}

static void TimeString(char *string, unsigned char hours, unsigned char minutes, unsigned char seconds) {
	 
    string[0] = (hours / (unsigned char)10 + '0');
    string[1] = (hours % (unsigned char)10 + '0');
    string[2] = ':';
    string[3] = (minutes / (unsigned char)10 + '0');
    string[4] = (minutes % (unsigned char)10 + '0');
    string[5] = ':';
    string[6] = (seconds / (unsigned char)10  + '0');
    string[7] = (seconds % (unsigned char)10 + '0');
    string[8] = '\0';
}