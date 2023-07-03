#include "app_clock.h"

#define MESSAGE     0
#define ALARM       1
#define DATE        2
#define TIME        3
#define CLEAR       4
#define PRINT       5

static void SaveTime( void );
static void SaveDate( void );
static void SaveAlarm( void );
static void ClearStorage( void );
static void UpdateAndPrint( void );

extern RTC_HandleTypeDef    RtcHandler;
RTC_HandleTypeDef           RtcHandler = {0};
//APP_MsgTypeDef       DataStorage;

extern RTC_TimeTypeDef  sTime;
RTC_TimeTypeDef         sTime = {0};
extern RTC_DateTypeDef  sDate;
RTC_DateTypeDef         sDate = {0};
static uint8_t          dateYearH;
extern RTC_AlarmTypeDef sAlarm;
RTC_AlarmTypeDef        sAlarm = {0};

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

static void SaveTime( void ) {
    sTime.Hours   = DataStorage.tm.tm_hour;
    sTime.Minutes = DataStorage.tm.tm_min;
    sTime.Seconds = DataStorage.tm.tm_sec;
    HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN);
}

static void SaveDate( void ) {
    sDate.Date = DataStorage.tm.tm_mday;
    sDate.Month = DataStorage.tm.tm_mon;
    sDate.Year = DataStorage.tm.tm_year % ( uint32_t ) 100;
    dateYearH = DataStorage.tm.tm_year / ( uint32_t ) 100;
    HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN);
}

static void SaveAlarm( void ) {
    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours = DataStorage.tm.tm_hour_a;
    sAlarm.AlarmTime.Minutes = DataStorage.tm.tm_min_a;
    HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BIN );
}

static void ClearStorage( void ) {
    APP_MsgTypeDef nullMessageStruct = {0};
    DataStorage = nullMessageStruct;
}

static void UpdateAndPrint( void ) {
    HAL_RTC_GetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN );
    HAL_RTC_GetAlarm( &RtcHandler, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );
    ( void ) printf("Time: %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
    ( void ) printf("Date: %02d/%02d/%02d%02d\r\n", sDate.Date, sDate.Month, dateYearH, sDate.Year);
    ( void ) printf("Alarm: %02d:%02d\r\n", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
}