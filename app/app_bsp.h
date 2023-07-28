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
    #include "stm32g0xx_hal_rcc.h"
    #include "stm32g0xx_hal_rcc_ex.h"
    #include "stm32g0xx_hal_pwr.h"
    #include "stm32g0xx_hal_pwr_ex.h"
    #include "stm32g0xx_hal_wwdg.h"
    #include "stm32g0xx_hal_spi.h"
    #include "stm32g0xx_hal_spi_ex.h"
    #include "hil_queue.h"
    #include <stdint.h>
    #include <string.h>

extern FDCAN_HandleTypeDef  CANHandler;

/*macro to detect erros, wehere if expr is evaluated to false is an error*/
#define assert_error(expr, error)         ((expr) ? (void)0U : Safe_State((uint8_t *)__FILE__, __LINE__, (error)))

extern void Safe_State(uint8_t *file, uint32_t line, uint8_t error);

/**
  * @brief   Enum that defines the types of errors of every module initialization.
  */

/*cppcheck-suppress misra-c2012-2.4 ; Enum used for functional safety.*/
typedef enum _App_ErrorsCode {
    WWDG_RET_ERROR = 1U,        /*!< WDG ERROR      1*/
    PWR_RET_ERROR,              /*!< PWR ERROR      2*/
    RCC_RET_ERROR,              /*!< RCC ERROR      3*/
    HARDFAULT_RET_ERROR,        /*!< HFAULT ERROR   4*/
    ECC_RET_ERROR,              /*!< ECC ERROR      5*/
    CAN_RET_ERROR,              /*!< CAN ERROR      6*/
    RTC_RET_ERROR,              /*!< RTC ERROR      7*/
    SPI_RET_ERROR,              /*!< SPI ERROR      8*/
    LCD_RET_ERROR,              /*!< LCD ERROR      9*/
    HAL_RET_ERROR,              /*!< HAL ERROR      10*/
    CAN_FUNC_ERROR,             /*!< CAN F ERROR    11*/
    HARDFAULT_FUNC_ERROR,       /*!< HFAULT ERROR   12*/
    LCD_PAR_ERROR,              /*!< LCD ERROR      13*/
    SPI_FUNC_ERROR,             /*!< LCD ERROR      14*/
    WWDG_FUNC_ERROR,            /*!< LCD ERROR      15*/
    ECC_FUNC_ERROR              /*!< LCD ERROR      16*/
} 
/*cppcheck-suppress misra-c2012-2.3 ; Macro required for functional safety.*/
App_ErrorsCode;

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
    uint32_t tm_sec;         /*!< Seconds,  range 0 to 59.*/
    uint32_t tm_min;         /*!< Minutes, range 0 to 59.*/
    uint32_t tm_hour;        /*!< Hours, range 0 to 23.*/
    uint32_t tm_min_a;       /*!< Alarm hours, range 0 to 23.*/
    uint32_t tm_hour_a;      /*!< Alarm minutes, range 0 to 59.*/
    uint32_t tm_mday;        /*!< Day of the month, range 1 to 31.*/
    uint32_t tm_mon;         /*!< Month, range 0 to 11.*/
    uint32_t tm_year;        /*!< Years, range 1900 to 2100.*/
    uint32_t tm_wday;        /*!< Day of the week, range 0 to 6.*/
    uint32_t tm_yday;        /*!< Day in the year, range 0 to 365.*/
    uint32_t tm_isdst;       /*!< Daylight saving time.*/
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

/**
  * @brief  Struct variable of memeberst to transmit to LCD 
  */
extern APP_MsgTypeDef ClockMsg;

typedef struct _NEW_MsgTypeDef 
{
  uint8_t data[8];
} NEW_MsgTypeDef;

#endif