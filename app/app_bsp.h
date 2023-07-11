/**
 * @file    app_bsp.h
 * @brief   **This file contains the resources required by the entire project folder.**
 *
 * Here are included all of the libraries that will be used, the headers of every file, and the structs
 * that will be read & written by multiple files.
 */

#ifndef BSP_H_
#define BSP_H_

    #include "stm32g0xx.h"
    #include "stm32g0xx_hal_gpio.h"
    #include "stm32g0xx_hal_fdcan.h"
    #include "stm32g0xx_hal_rtc.h"
    #include "stm32g0xx_hal_rtc_ex.h"
    #include "stm32g0xx_hal_pwr.h"
    #include "stm32g0xx_hal_pwr_ex.h"
    #include "stm32g0xx_hal_wwdg.h"
    #include <stdint.h>
    /* cppcheck-suppress misra-c2012-21.6 ; The library is only for testing pourpose. */
    #include <stdio.h>

extern FDCAN_HandleTypeDef  CANHandler;

/**
  * @brief   Enum that defines which type of message is received by the CAN bus.
  */

typedef enum
{
    SERIAL_MSG_NONE,
    SERIAL_MSG_TIME,
    SERIAL_MSG_DATE,
    SERIAL_MSG_ALARM
} APP_Messages;

/**
  * @brief   Structure that contains the values received by the CAN bus and that will be used to set the RTC.
  */

typedef struct _APP_TmTypeDef 
{
    uint32_t tm_sec;         /*!< Seconds,  range 0 to 59          */
    uint32_t tm_min;         /*!< Minutes, range 0 to 59           */
    uint32_t tm_hour;        /*!< Hours, range 0 to 23             */
    uint32_t tm_min_a;       /*!< Alarm minutes, range 0 to 59     */
    uint32_t tm_hour_a;      /*!< Alarm hours, range 0 to 59       */
    uint32_t tm_mday;        /*!< Day of the month, range 1 to 31  */
    uint32_t tm_mon;         /*!< Month, range 0 to 11             */
    uint32_t tm_year;        /*!< Years in rage 1900 2100          */
    uint32_t tm_wday;        /*!< Day of the week, range 0 to 6    */
    uint32_t tm_yday;        /*!< Day in the year, range 0 to 365  */
    uint32_t tm_isdst;       /*!< Daylight saving time             */
} APP_TmTypeDef;

/**
  * @brief   Structure that contains the type of message received and it's values.
  */

typedef struct _APP_MsgTypeDef  
{
    uint8_t msg;          /*!< Store the message type to send */
    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */
} APP_MsgTypeDef;

extern APP_MsgTypeDef   DataStorage;
extern APP_Messages     MessageType;

#endif