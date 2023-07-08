/**
 * @file    app_clock.c
 * @brief   **File that manages the functioning of the RTC settings & display.**
 *
 * This file uses the data obtained and validated in the app_serial.c file to configure the RTC module 
 * and later reflect its values ​​on the display.
 */

#include "app_clock.h"

/** 
  * @defgroup ClockStates
  @{ */
#define MESSAGE     0
#define ALARM       1
#define DATE        2
#define TIME        3
#define CLEAR       4
#define PRINT       5
/**
  @} */

static void SaveTime( void );
static void SaveDate( void );
static void SaveAlarm( void );
static void ClearStorage( void );
static void UpdateAndPrint( void );

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
 * @brief   **Function that initialices the registers of the RTC module.**
 *
 * The RTC is set to the 24 hr format. 
 *
 * @param   <RtcHandler[out]> Struct-type variable to save the configuration of the RTC registers.
 * @param   <sTime[out]> Struct-type variable to set the initial values of the time registers.
 * @param   <sDate[out]> Struct-type variable to set the initial values of the date registers.
 * @param   <sAlarm[out]> Struct-type variable to set the initial values of the alarm registers.
 */

void Clock_Init( void )
{
    RtcHandler.Instance          = RTC;
    RtcHandler.Init.HourFormat   = RTC_HOURFORMAT_24;
    RtcHandler.Init.AsynchPrediv = 127;
    RtcHandler.Init.SynchPrediv  = 255;
    RtcHandler.Init.OutPut       = RTC_OUTPUT_DISABLE;

    HAL_RTC_Init( &RtcHandler );

    RTC_DateTypeDef sDate = {0};
    RTC_TimeTypeDef sTime = {0};
    RTC_AlarmTypeDef sAlarm = {0};

    sTime.Hours          = 0x12;
    sTime.Minutes        = 0x00;
    sTime.Seconds        = 0x00;
    sTime.SubSeconds     = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;


    HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BCD );

    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x16;
    sDate.Year = 0x99;
    HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BCD );

    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours   = 0x12;
    sAlarm.AlarmTime.Minutes = 0x00;
    HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BCD );
}

/**
 * @brief   **Functon with the state machine that manages the Clock set & display.**
 *
 * The state machine calls functions that passes the new values of time, date and alarm that come through
 * the CAN bus to the RTC, then clears the struct that was readed, and finally, displays this data 
 * every second. More details are shown in the diagram.
 */

void Clock_Task( void )
{
    static uint8_t StateClock = MESSAGE;
    static uint8_t UpdateFlag = 0;
    static uint32_t TickStartClock;

    switch( StateClock )
    {
        case MESSAGE:
            if(DataStorage.msg == ( uint8_t ) SERIAL_MSG_NONE)
            {
                StateClock = PRINT;
            }
            if(DataStorage.msg == ( uint8_t ) SERIAL_MSG_ALARM)
            {
                StateClock = ALARM;
            }
            if(DataStorage.msg == ( uint8_t ) SERIAL_MSG_DATE)
            {
                StateClock = DATE;
            }
            if(DataStorage.msg == ( uint8_t ) SERIAL_MSG_TIME)
            {
                StateClock = TIME;
            }
        break;

        case ALARM:
            SaveAlarm();
            UpdateFlag = 1;
            StateClock = CLEAR;
        break;

        case DATE:
            SaveDate();
            UpdateFlag = 1;
            StateClock = CLEAR;
        break;

        case TIME:
            SaveTime();
            UpdateFlag = 1;
            StateClock = CLEAR;
        break;

        case CLEAR:
            ClearStorage();
            StateClock = PRINT;
        break;

        case PRINT:
            if( ( ( HAL_GetTick() - TickStartClock ) >= 1000 ) || ( UpdateFlag == ( uint8_t ) 1 ) )
            {
                TickStartClock = HAL_GetTick();
                UpdateFlag = 0;
                UpdateAndPrint();
            }
            StateClock = MESSAGE;
        break;

        default:
        break;
    }
}

/**
 * @brief   **Function that passes time values to the RTC module.**
 *
 * The function reads the struct DataStorage to assign it's values to the strcut that feeds the RTC
 * time initial values.
 */

static void SaveTime( void ) {
    sTime.Hours   = DataStorage.tm.tm_hour;
    sTime.Minutes = DataStorage.tm.tm_min;
    sTime.Seconds = DataStorage.tm.tm_sec;
    HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN);
}

/**
 * @brief   **Function that passes date values to the RTC module.**
 *
 * The function reads the struct DataStorage to assign it's values to the strcut that feeds the RTC
 * date initial values.
 */

static void SaveDate( void ) {
    sDate.Date = DataStorage.tm.tm_mday;
    sDate.Month = DataStorage.tm.tm_mon;
    sDate.Year = DataStorage.tm.tm_year % ( uint32_t ) 100;
    dateYearH = DataStorage.tm.tm_year / ( uint32_t ) 100;
    HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN);
}

/**
 * @brief   **Function that passes alarm values to the RTC module.**
 *
 * The function reads the struct DataStorage to assign it's values to the strcut that feeds the RTC
 * alarm initial values.
 */

static void SaveAlarm( void ) {
    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours = DataStorage.tm.tm_hour_a;
    sAlarm.AlarmTime.Minutes = DataStorage.tm.tm_min_a;
    HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BIN );
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
    HAL_RTC_GetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN );
    HAL_RTC_GetAlarm( &RtcHandler, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );
    ( void ) printf("Time: %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
    ( void ) printf("Date: %02d/%02d/%02d%02d\r\n", sDate.Date, sDate.Month, dateYearH, sDate.Year);
    ( void ) printf("Alarm: %02d:%02d\r\n", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
}