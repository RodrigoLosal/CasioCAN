#include "app_clock.h"

#define MESSAGE     00
#define ALARM       10
#define DATE        11
#define TIME        12
#define CLEAR       20
#define PRINT       30

RTC_HandleTypeDef RtcHandler = {0};
APP_MsgTypeDef nullMessageStruct = {0};
extern APP_MsgTypeDef DataStorage;


static uint8_t state = MESSAGE;
uint8_t aux;

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
    RTC_DateTypeDef sDate   = {0};
    RTC_AlarmTypeDef sAlarm = {0};
    RTC_TimeTypeDef sTime   = {0};
    static uint32_t tickstart;
    static uint8_t dateYearH = 0;

    switch(state)
    {
        case MESSAGE:
            if(DataStorage.msg == SERIAL_MSG_NONE)
            {
                state = PRINT;
            }
            if(DataStorage.msg == SERIAL_MSG_ALARM)
            {
                state = ALARM;
            }

            if(DataStorage.msg == SERIAL_MSG_DATE)
            {
                state = DATE;
            }

            if(DataStorage.msg == SERIAL_MSG_TIME)
            {
                state = TIME;
            }

        break;

        case ALARM:
            sAlarm.Alarm = RTC_ALARM_A;
            sAlarm.AlarmTime.Hours = DataStorage.tm.tm_hour_a;
            sAlarm.AlarmTime.Minutes = DataStorage.tm.tm_min_a;
            HAL_RTC_SetAlarm( &RtcHandler, &sAlarm, RTC_FORMAT_BIN );
            state = CLEAR;
        break;

        case DATE:
            sDate.Date = DataStorage.tm.tm_mday;
            sDate.Month = DataStorage.tm.tm_mon;
            sDate.Year = DataStorage.tm.tm_year % 100;
            dateYearH = DataStorage.tm.tm_year / 100;
            HAL_RTC_SetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN);
            state = CLEAR;
        break;

        case TIME:
            sTime.Hours   = DataStorage.tm.tm_hour;
            sTime.Minutes = DataStorage.tm.tm_min;
            sTime.Seconds = DataStorage.tm.tm_sec;
            HAL_RTC_SetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN);
            state = CLEAR;
        break;

        case CLEAR:
            DataStorage = nullMessageStruct;
            state = PRINT;
        break;

        case PRINT:
            if( (HAL_GetTick() - tickstart) >= 1000 )
            {
                tickstart = HAL_GetTick();
                HAL_RTC_GetTime( &RtcHandler, &sTime, RTC_FORMAT_BIN );
                HAL_RTC_GetDate( &RtcHandler, &sDate, RTC_FORMAT_BIN );
                HAL_RTC_GetAlarm( &RtcHandler, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );
                printf("Time: %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
                printf("Date: %02d/%02d/%02d%02d\r\n", sDate.Date, sDate.Month, dateYearH, sDate.Year);
                printf("Alarm: %02d:%02d\r\n", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
            }
            state = MESSAGE;
        break;
    }
}