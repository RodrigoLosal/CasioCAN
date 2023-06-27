#include "app_clock.h"

#define MESSAGE     1
#define ALARM       2
#define DATE        3
#define TIME        4
#define CLEAR       5
#define PRINT       6

void SaveTime( void );
void SaveDate( void );
void SaveAlarm( void );
void ClearStorage( void );
void UpdateAndPrint( void );

RTC_HandleTypeDef       RtcHandler;
extern APP_MsgTypeDef   DataStorage;

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
static uint8_t dateYearH;
RTC_AlarmTypeDef sAlarm;

static uint8_t State = MESSAGE;

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
    static uint32_t tickstart;

    switch(State)
    {
        case MESSAGE:
            if(DataStorage.msg == SERIAL_MSG_NONE)
            {
                State = PRINT;
            }
            if(DataStorage.msg == SERIAL_MSG_ALARM)
            {
                State = ALARM;
            }
            if(DataStorage.msg == SERIAL_MSG_DATE)
            {
                State = DATE;
            }
            if(DataStorage.msg == SERIAL_MSG_TIME)
            {
                State = TIME;
            }
        break;

        case ALARM:
            SaveAlarm();
            State = CLEAR;
        break;

        case DATE:
            SaveDate();
            State = CLEAR;
        break;

        case TIME:
            SaveTime();
            State = CLEAR;
        break;

        case CLEAR:
            ClearStorage();
            State = PRINT;
        break;

        case PRINT:
            if( (HAL_GetTick() - tickstart) >= 1000 )
            {
                tickstart = HAL_GetTick();
                UpdateAndPrint();
            }
            State = MESSAGE;
        break;
    }
}

void SaveTime( void ) {
    sTime.Hours   = DataStorage.tm.tm_hour;
    sTime.Minutes = DataStorage.tm.tm_min;
    sTime.Seconds = DataStorage.tm.tm_sec;
    HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN);
}

void SaveDate( void ) {
    sDate.Date = DataStorage.tm.tm_mday;
    sDate.Month = DataStorage.tm.tm_mon;
    sDate.Year = DataStorage.tm.tm_year % 100;
    dateYearH = DataStorage.tm.tm_year / 100;
    HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN);
}

void SaveAlarm( void ) {
    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours = DataStorage.tm.tm_hour_a;
    sAlarm.AlarmTime.Minutes = DataStorage.tm.tm_min_a;
    HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BIN );
}

void ClearStorage( void ) {
    APP_MsgTypeDef nullMessageStruct = {0};
    DataStorage = nullMessageStruct;
}

void UpdateAndPrint( void ) {
    HAL_RTC_GetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN );
    HAL_RTC_GetAlarm( &RtcHandler, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );
    printf("Time: %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
    printf("Date: %02d/%02d/%02d%02d\r\n", sDate.Date, sDate.Month, dateYearH, sDate.Year);
    printf("Alarm: %02d:%02d\r\n", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
}