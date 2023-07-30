/**
 * @file    app_clock.c
 * @brief   **File that manages the functioning of the RTC settings & display.**
 *
 * This file uses the data obtained and validated in the app_serial.c file to configure the RTC module 
 * and later reflect its values ​​on the display.
 */

#include "app_clock.h"

/** 
  * @defgroup Clock States.
  @{*/
#define IDLE        0 /*!< First state of the clock states.*/
#define RECEPTION   1 /*!< Second state of the clock states.*/
#define ALARM       2 /*!< Third state of the clock states.*/
#define DATE        3 /*!< Fourth state of the clock states.*/
#define TIME        4 /*!< Fifth state of the clock states.*/
#define CLEAR       5 /*!< Sixth state of the clock states.*/
#define MESSAGE     6 /*!< Seventh state of the clock states.*/
/**@}*/

static void SaveTime( void );
static void SaveDate( void );
static void SaveAlarm( void );
static void ClearStorage( void );
static void UpdateAndPrint( void );
static uint32_t Clock_Machine( uint32_t currentState ); 

/**
  * @brief   Structure that will contain the values to initialice the RTC.
  */

extern RTC_HandleTypeDef    RtcHandler;
RTC_HandleTypeDef           RtcHandler = {0};

/**
  * @brief   Structure that will contain the values to initialice the RTC time values.
  */

extern RTC_TimeTypeDef  sTime;
RTC_TimeTypeDef         sTime = {0};

/**
  * @brief   Structure that will contain the values to initialice the RTC date values.
  */

extern RTC_DateTypeDef  sDate;
RTC_DateTypeDef         sDate = {0};

/**
  * @brief   Variable that will contain the first two digits of a given year.
  */

static uint8_t          dateYearH;

/**
  * @brief   Structure that will contain the values to initialice the RTC alarm values.
  */

extern RTC_AlarmTypeDef sAlarm;
RTC_AlarmTypeDef        sAlarm = {0};

/**
 * @brief This is a struct variable to contain the members of date and time.
 */ 

APP_MsgTypeDef ClockMsg = {0};

/**
 * @brief Struct variable of Queue elements
*/
QUEUE_HandleTypeDef ClockQueue = {0};

/**
 * @brief Struct variable with the array of Queue
*/
/* cppcheck-suppress misra-c2012-8.7 ;If header is modified the program will not work*/
NEW_MsgTypeDef buffer_clock[45];  /* cppcheck-suppress misra-c2012-8.4 ;Its been used due to the queue*/


/**
 * @brief   **Function that initialices the registers of the RTC module.**
 *
 * The RTC is set to the 24 hr format. 
 */

void Clock_Init( void )
{
    ClockQueue.Buffer = (void*)buffer_clock;    /*Indicate the buffer that the tail will use as memory space*/
    ClockQueue.Elements = 45u;                  /*Indicate the maximum number of elements that can be stored*/ 
    ClockQueue.Size = sizeof( NEW_MsgTypeDef ); /*Indicate the size in bytes of the type of elements to handle*/ 
    HIL_QUEUE_Init( &ClockQueue );              /*Initialize the queue*/ 

    HAL_StatusTypeDef Status;

    RtcHandler.Instance          = RTC;
    RtcHandler.Init.HourFormat   = RTC_HOURFORMAT_24;
    RtcHandler.Init.AsynchPrediv = 127;
    RtcHandler.Init.SynchPrediv  = 255;
    RtcHandler.Init.OutPut       = RTC_OUTPUT_DISABLE;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_Init( &RtcHandler );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    RTC_DateTypeDef sDate = {0};
    RTC_TimeTypeDef sTime = {0};
    RTC_AlarmTypeDef sAlarm = {0};

    sTime.Hours          = 0x12;
    sTime.Minutes        = 0x00;
    sTime.Seconds        = 0x00;
    sTime.SubSeconds     = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BCD );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x01;
    sDate.Year = 0x00;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BCD );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours   = 0x12;
    sAlarm.AlarmTime.Minutes = 0x00;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BCD );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
}

/**
* @brief Clock task function 
* This function checks the queue of pending tasks every 50ms and process them calling the clock machine   
*/

void Clock_Task(void) {
   static uint32_t state = RECEPTION;
   static uint32_t serialtick =0;
   /* We check the waiting queue with 50ms */
   if ((HAL_GetTick() - serialtick) >= 50u) {
       serialtick = HAL_GetTick(); 
       
        state = Clock_Machine(state);
   }
}

/**
 * @brief   **Functon with the state machine that manages the Clock set & display.**
 *
 * The state machine calls functions that passes the new values of time, date and alarm that come through
 * the CAN bus to the RTC, then clears the struct that was readed, and finally, displays this data 
 * every second. More details are shown in the diagram.
 */

static uint32_t Clock_Machine( uint32_t currentState )
{
    uint32_t StateClock = currentState;

    switch( StateClock )
    {
        case IDLE:
            StateClock = RECEPTION;
        break;

        case RECEPTION:
            /*Revision and unpaked the messages */
           if( HIL_QUEUE_IsEmptyISR( &ClockQueue, 0xFF ) == ( uint8_t ) 0 )
            {
                /*Read the first message*/
                (void)HIL_QUEUE_ReadISR( &ClockQueue, &DataStorage, 0xFF );

                if(DataStorage.msg == (uint8_t)SERIAL_MSG_TIME) {
                    StateClock = TIME;
                }
                else if(DataStorage.msg == (uint8_t)SERIAL_MSG_DATE) {
                    StateClock = DATE;
                }
                else if(DataStorage.msg == (uint8_t)SERIAL_MSG_ALARM) {
                    StateClock = ALARM;
                }
                else{    
                }
            }
            else{ 
                StateClock = MESSAGE;
            }
        break;

        case ALARM:
            SaveAlarm();
            StateClock = CLEAR;
        break;

        case DATE:
            SaveDate();
            StateClock = CLEAR;
        break;

        case TIME:
            SaveTime();
            StateClock = CLEAR;
        break;

        case CLEAR:
            ClearStorage();
            StateClock = MESSAGE;
        break;

        case MESSAGE:
            UpdateAndPrint();

            StateClock = IDLE;
        break;

        default:
        break;
    }

    return StateClock;
}

/**
 * @brief   **Function that passes time values to the RTC module.**
 *
 * The function reads the struct DataStorage to assign it's values to the strcut that feeds the RTC
 * time initial values.
 */

static void SaveTime( void ) {
    HAL_StatusTypeDef Status;

    sTime.Hours   = DataStorage.tm.tm_hour;
    sTime.Minutes = DataStorage.tm.tm_min;
    sTime.Seconds = DataStorage.tm.tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN);
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
}

/**
 * @brief   **Function that passes date values to the RTC module.**
 *
 * The function reads the struct DataStorage to assign it's values to the strcut that feeds the RTC
 * date initial values.
 */

static void SaveDate( void ) {
    HAL_StatusTypeDef Status;

    sDate.WeekDay = DataStorage.tm.tm_wday;
    sDate.Date = DataStorage.tm.tm_mday;
    sDate.Month = DataStorage.tm.tm_mon;
    sDate.Year = DataStorage.tm.tm_year % ( uint32_t ) 100;
    dateYearH = DataStorage.tm.tm_year / ( uint32_t ) 100;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN);
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
}

/**
 * @brief   **Function that passes alarm values to the RTC module.**
 *
 * The function reads the struct DataStorage to assign it's values to the strcut that feeds the RTC
 * alarm initial values.
 */

static void SaveAlarm( void ) {
    HAL_StatusTypeDef Status;

    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours = DataStorage.tm.tm_hour_a;
    sAlarm.AlarmTime.Minutes = DataStorage.tm.tm_min_a;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BIN );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
}

/**
 * @brief   **Function that sets the entire DataStorage struct to 0.**
 *
 * The function clears all of the existing values of the DataStorage struct.
 */

static void ClearStorage( void ) {
    APP_MsgTypeDef nullMessageStruct = {0};
    DataStorage = nullMessageStruct;
}


/**
 * @brief   **Function to set & display the RTC.**
 *
 * The values that were saved in the previous functions are now used to set the new values of the RTC,
 * and displays them every second afterwards.
 */

static void UpdateAndPrint( void ) {
    HAL_StatusTypeDef Status;

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_GetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );


    /*The function is used and its result is verified.*/
    Status = HAL_RTC_GetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*The function is used and its result is verified.*/
    Status = HAL_RTC_GetAlarm( &RtcHandler, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    ClockMsg.tm.tm_hour = sTime.Hours;
    ClockMsg.tm.tm_min = sTime.Minutes;
    ClockMsg.tm.tm_sec = sTime.Seconds;

    ClockMsg.tm.tm_mday = sDate.Date;
    ClockMsg.tm.tm_mon = sDate.Month;
    ClockMsg.tm.tm_year = ( ( ( uint32_t ) dateYearH * ( uint32_t ) 100 ) + ( uint32_t ) sDate.Year );
    ClockMsg.tm.tm_wday = sDate.WeekDay;

    (void) HIL_QUEUE_WriteISR( &DisplayQueue, &DataStorage, 0xFF );

    ClockMsg.msg = 1;
}