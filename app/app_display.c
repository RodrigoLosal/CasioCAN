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
#define IDLE        1 /*!< State one of clock state machine.*/
#define RECEPTION   2 /*!< State two of clock state machine.*/
#define TRANSMIT    3 /*!< State three of clock state machine.*/
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
static void Display_Machine( void );

static char TimeArray[9];
static char DateArray[16];

/**
 * @brief  Pointer to the address of the string that contains the time format.
 */

extern char *TimeArrayPtr;
char *TimeArrayPtr = TimeArray;

/**
 * @brief  Pointer to the address of the string that contains the date format.
 */

extern char *DateArrayPtr;
char *DateArrayPtr = DateArray;

/**
 * @brief Struct variable of Queue elements
*/

QUEUE_HandleTypeDef DisplayQueue = {0};    

/**
 * @brief Struct variable with the array of Queue
*/

/* cppcheck-suppress misra-c2012-8.7 ;If header is modified the program will not work*/
NEW_MsgTypeDef buffer_display[90];  /* cppcheck-suppress misra-c2012-8.4 ;Its been used due to the queue*/

void Display_Init( void ) {
    DisplayQueue.Buffer = (void*)buffer_display;  /*Indicate the buffer that the tail will use as memory space*/
    DisplayQueue.Elements = 90u;                  /*Indicate the maximum number of elements that can be stored*/ 
    DisplayQueue.Size = sizeof( NEW_MsgTypeDef ); /*Indicate the size in bytes of the type of elements to handle*/
    HIL_QUEUE_Init( &DisplayQueue );              /*Initialize the queue*/
    HAL_StatusTypeDef Status;

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

    /*The function is used and its result is verified.*/
    Status = HAL_SPI_Init( &SPI_Handler );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, SPI_RET_ERROR );

    (void)HEL_LCD_Init(&hLcd);
}


/**
* @brief Display task function 
* This function checks the queue of pending tasks every 100ms and process them calling the display machine   
*/
void Display_Task(void) {
   static uint32_t serialtick =0;

   if ((HAL_GetTick() - serialtick) >= 100u) {
       serialtick = HAL_GetTick(); 
       
        Display_Machine();
   }
}

static void Display_Machine( void ) {
    switch(state_lcd){
        case IDLE:
            state_lcd = RECEPTION;
        break;

        case RECEPTION:
            /*Revision and unpaked the messages */
           if( HIL_QUEUE_IsEmptyISR( &DisplayQueue, 0xFF ) == 0u )
            {
                /*Read the first message*/
                (void)HIL_QUEUE_ReadISR( &DisplayQueue, &ClockMsg, 0xFF);

                state_lcd = TRANSMIT;               
            }
            else 
            {
                state_lcd = IDLE;
            }       
        break;

        case TRANSMIT:
            (void)HEL_LCD_SetCursor( &hLcd, 1, 3);
            TimeString( TimeArrayPtr, ClockMsg.tm.tm_hour, ClockMsg.tm.tm_min, ClockMsg.tm.tm_sec );
            (void)HEL_LCD_String( &hLcd, TimeArrayPtr);
            
            (void)HEL_LCD_SetCursor( &hLcd, 0, 1);
            DateString(DateArrayPtr, ClockMsg.tm.tm_mon, ClockMsg.tm.tm_mday, ClockMsg.tm.tm_year, ClockMsg.tm.tm_wday);
            (void)HEL_LCD_String( &hLcd, DateArrayPtr);
            state_lcd = IDLE;
        break;

        default:
        break;
    }
}

static void DateString(char *string, unsigned char month, unsigned char day, unsigned short year, unsigned char weekday) {
    char WeekDays[7][4] = { [0] = "Mo", [1] = "Tu", [2] = "We", [3] = "Th", [4] = "Fr", [5] = "Sa", [6] = "Su"};
    char Months[12][4] = { [0] = "Jan", [1] = "Feb", [2] = "Mar", [3] = "Apr", [4] = "May", [5] = "Jun", [6] = "Jul", [7] = "Aug", [8] = "Sep", [9] = "Oct", [10] = "Nov", [11] = "Dec"};
    
    ( void ) strcpy( &string[0], Months[ month - ( unsigned char ) 1] ); 
    string[3] = ',';
    string[4] = (day / ( unsigned char ) 10 + '0');
    string[5] = (day % ( unsigned char ) 10 + '0');
    string[6] = ' ';
    string[7] = ( char ) ( ( ( int ) year / 1000 ) + '0');
    string[8] = ( char ) ( ( int ) year % ( int ) 1000 / ( int ) 100 + '0');
    string[9] = ( char ) ( ( int ) year % ( int ) 1000 % ( int ) 100 / ( int ) 10 + '0');
    string[10] = ( char ) ( ( int ) year % ( int ) 1000 % ( int ) 100 % ( int ) 10 / ( int ) 1 + '0');
    string[11] = ' ';
    ( void ) strcpy( &string[12], WeekDays [ weekday - ( unsigned char ) 1 ] );
    string[14] = '\0';
}

static void TimeString(char *string, unsigned char hours, unsigned char minutes, unsigned char seconds) {
	 
    string[0] = ( hours / (unsigned char)10 + '0' );
    string[1] = ( hours % (unsigned char)10 + '0' );
    string[2] = ':';
    string[3] = ( minutes / (unsigned char)10 + '0' );
    string[4] = ( minutes % (unsigned char)10 + '0' );
    string[5] = ':';
    string[6] = ( seconds / (unsigned char)10  + '0' );
    string[7] = ( seconds % (unsigned char)10 + '0' );
    string[8] = '\0';
}