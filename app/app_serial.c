#include "app_serial.h"
#include "app_bsp.h"
#include <stdio.h>

#define IDLE    1
#define MESSAGE 2
#define TIME    3
#define DATE    4
#define ALARM   5
#define ERROR   6
#define OK      7

#define JAN 1
#define FEB 2
#define MAR 3
#define APR 4
#define MAY 5
#define JUN 6
#define JUL 7
#define AUG 8
#define SEP 9
#define OCT 10
#define NOV 11
#define DEC 12

uint8_t HexToBCD(uint8_t Data);
uint8_t TimeValidaton( uint8_t *Data );
uint8_t DateValidaton( uint8_t *Data );
uint8_t WeekDay( uint8_t *Data );
uint16_t YearDay(uint8_t *Data );
uint8_t DaylightSavingTime( uint8_t *Data );
uint8_t AlarmValidaton( uint8_t *Data );
static void CanTp_SingleFrameTx( uint8_t *Data, uint8_t Size );
static uint8_t CanTp_SingleFrameRx( uint8_t *Data, uint8_t *Size );

FDCAN_HandleTypeDef     CANHandler;
FDCAN_RxHeaderTypeDef   CANRxHeader;
FDCAN_TxHeaderTypeDef   CANTxHeader;
FDCAN_FilterTypeDef     CANFilter;
APP_MsgTypeDef          DataStorage;
APP_Messages            MessageType;

uint8_t RxData[8];
uint8_t MessageData[7];
uint8_t MessageSize;
uint8_t Message = 0;

uint8_t State = IDLE;

void Serial_Init( void )
{
    CANHandler.Instance                     = FDCAN1;
    CANHandler.Init.Mode                    = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat             = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider            = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode         = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.NominalPrescaler        = 10;
    CANHandler.Init.NominalSyncJumpWidth    = 1;
    CANHandler.Init.NominalTimeSeg1         = 11;
    CANHandler.Init.NominalTimeSeg2         = 4;
    CANHandler.Init.StdFiltersNbr           = 1;
    HAL_FDCAN_Init( &CANHandler);

    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;

    CANTxHeader.Identifier  = 0x122;

    CANFilter.IdType       = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex  = 0;
    CANFilter.FilterType   = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1    = 0x111;
    CANFilter.FilterID2    = 0x7FF;
    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );

    HAL_FDCAN_ConfigGlobalFilter( &CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE );

    HAL_FDCAN_Start( &CANHandler);

    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );
}

void Serial_Task( void )
{
    uint8_t MessageOK    = 0x55;
    uint8_t MessageERROR = 0xAA;

    switch( State ) {
        case IDLE:
            if( CanTp_SingleFrameRx( MessageData, &MessageSize ) ) {
                State = MESSAGE;
            }
        break;

        case MESSAGE:
            printf("Entramos a MESSAGE.\n\r");
            if( MessageData[0] == SERIAL_MSG_TIME ) {
                State = TIME;
            }
            else if( MessageData[0] == SERIAL_MSG_DATE ) {
                State = DATE;
            }
            else if( MessageData[0] == SERIAL_MSG_ALARM ) {
                State = ALARM;
            }
            else {
                State = ERROR;
            }
        break;

        case TIME:
            printf("Vas a configurar hora.\n\r");
            if( TimeValidaton( MessageData ) ) {
                printf("Hora: %u\n\r", ( unsigned int ) DataStorage.tm.tm_hour );
                printf("Minutos: %u\n\r", ( unsigned int ) DataStorage.tm.tm_min );
                printf("Segundos: %u\n\r", ( unsigned int ) DataStorage.tm.tm_sec );
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case DATE:
            printf("Vas a configurar fecha.\n\r");
            if( DateValidaton( MessageData ) ) {
                printf("Dia: %u\n\r", ( unsigned int ) DataStorage.tm.tm_mday );
                printf("Mes: %u\n\r", ( unsigned int ) DataStorage.tm.tm_mon );
                printf("Anio: %u\n\r", ( unsigned int ) DataStorage.tm.tm_year );
                WeekDay( MessageData );
                printf("Weekday: %u\n\r", ( unsigned int ) DataStorage.tm.tm_wday );
                YearDay( MessageData );
                printf("Yearday: %u\n\r", ( unsigned int ) DataStorage.tm.tm_yday );
                DaylightSavingTime( MessageData );
                printf("Daylight Saving Time: %u\n\r", ( unsigned int ) DataStorage.tm.tm_isdst );
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ALARM:
            printf("Vas a configurar alarma.\n\r");
            if( AlarmValidaton( MessageData ) ) {
                printf("Hora de alarma: %u\n\r", ( unsigned int ) DataStorage.tm.tm_hour );
                printf("Minutos de alarma: %u\n\r", ( unsigned int ) DataStorage.tm.tm_min );
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ERROR:
            printf("ERROR.\n\r");
            DataStorage.msg = MessageERROR;
            CanTp_SingleFrameTx( &DataStorage.msg, 2 );
            State = IDLE;
        break;

        case OK:
            printf("OK.\n\r");
            DataStorage.msg = MessageOK;
            CanTp_SingleFrameTx( &DataStorage.msg, 2 );
            State = IDLE;
        break;
    }
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
  HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );
  Message = 1;
}

uint8_t HexToBCD( uint8_t Data ) {
    uint8_t DataBCD;

    DataBCD = ( ( ( Data >> 4 ) * 10 ) + ( Data & 0x0F ) );

    return DataBCD;
}

uint8_t TimeValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Hours = HexToBCD( *( Data + 1 ) );
    uint8_t Minutes = HexToBCD( *( Data + 2 ) );
    uint8_t Seconds = HexToBCD( *( Data + 3 ) );

    if( Hours >= 0 && Hours < 24 && Minutes >= 0 && Minutes < 60 && Seconds >= 0 && Seconds < 60) {
        DataStorage.tm.tm_hour = Hours;
        DataStorage.tm.tm_min = Minutes;
        DataStorage.tm.tm_sec = Seconds;
        Flag = 1;
    }
    else {
        Flag = 0;
    }
    return Flag;
}

uint8_t DateValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Day = HexToBCD( *( Data + 1 ) );
    uint8_t Month = HexToBCD( *( Data + 2 ) );
    uint16_t Year = ( HexToBCD( *( Data + 3 ) ) * 100 ) + HexToBCD( *( Data + 4 ) );

    if( Year > 1900 && Year < 2100 ) {
        DataStorage.tm.tm_year = Year;

        if( Month >= JAN && Month <= DEC ) {
            DataStorage.tm.tm_mon = Month;

            if( Month == JAN || Month == MAR || Month == MAY || Month == JUL || Month == AUG || Month == OCT || Month == DEC ) {
                if( Day >= 1 && Day <= 31 ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else if( Month == APR || Month == JUN || Month == SEP || Month == NOV ) {
                if( Day >= 1 && Day <= 30 ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else if( ( Year % 4 ) == 0 && Month == FEB ) {
                if( Day >=1 && Day <= 29 ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else if ( Month == FEB ) {
                if( Day >=1 && Day <=28 ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }
        }
        else {
            Flag = 0;
        }
    }
    else {
        Flag = 0;
    }
    return Flag;
}

uint8_t WeekDay( uint8_t *Data ) {
    uint8_t Day = HexToBCD( *( Data + 1 ) );
    uint8_t Month = HexToBCD( *( Data + 2 ) );
    uint16_t Year = ( HexToBCD( *( Data + 3 ) ) * 100 ) + HexToBCD( *( Data + 4 ) );
    uint16_t k, j , h;

    if (Month == JAN) {
        Month = 13;
        Year--;
    }
    if (Month == FEB) {
        Month = 14;
        Year--;
    }
    k = Year % 100;
    j = Year / 100;
    h = ( Day + 13 * ( Month + 1 ) / 5 + k + k / 4 + j / 4 + 5 * j ) % 7;

    DataStorage.tm.tm_wday = h;

    return h;
}

uint16_t YearDay(uint8_t *Data ) {
    uint8_t Day = HexToBCD( *( Data + 1 ) );
    uint8_t Month = HexToBCD( *( Data + 2 ) );
    uint16_t Year = ( HexToBCD( *( Data + 3 ) ) * 100 ) + HexToBCD( *( Data + 4 ) );
    uint16_t YearDay;

    switch( Month ) {
        case JAN:
            YearDay = Day;
        break;

        case FEB:
            YearDay = Day + 31;
        break;

        case MAR:
            YearDay = Day + 31 + 28;
        break;

        case APR:
            YearDay = Day + 31 + 28 + 31;
        break;

        case MAY:
            YearDay = Day + 31 + 28 + 31 + 30;
        break;

        case JUN:
            YearDay = Day + 31 + 28 + 31 + 30 + 31;
        break;

        case JUL:
            YearDay = Day + 31 + 28 + 31 + 30 + 31 +30;
        break;

        case AUG:
            YearDay = Day + 31 + 28 + 31 + 30 + 31 +30 + 31;
        break;

        case SEP:
            YearDay = Day + 31 + 28 + 31 + 30 + 31 +30 + 31 + 31;
        break;

        case OCT:
            YearDay = Day + 31 + 28 + 31 + 30 + 31 +30 + 31 + 31 + 30;
        break;

        case NOV:
            YearDay = Day + 31 + 28 + 31 + 30 + 31 +30 + 31 + 31 + 30 + 31;
        break;

        case DEC:
            YearDay = Day + 31 + 28 + 31 + 30 + 31 +30 + 31 + 31 + 30 + 31 + 30;
        break;
    }

    if( ( Year % 4 ) == 0 && Month > FEB) {
        YearDay += 1;
    }

    DataStorage.tm.tm_yday = YearDay;

    return YearDay;
}

uint8_t DaylightSavingTime( uint8_t *Data ) {
    uint8_t Flag;
    uint16_t Year = ( HexToBCD( *( Data + 3 ) ) * 100 ) + HexToBCD( *( Data + 4 ) );
    uint16_t DayNumber = YearDay( Data );
    uint16_t Start = 71, End = 309; //From March 12th to November 5th, 

    if( ( Year % 4 ) == 0 ) {
        Start++;
        End++;
    }

    if( DayNumber >= Start && DayNumber <= End ) {
        Flag = 1;
    }
    else {
        Flag = 0;
    }

    DataStorage.tm.tm_isdst = Flag;

    return Flag;
}

uint8_t AlarmValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Hours = HexToBCD( *( Data + 1 ) );
    uint8_t Minutes = HexToBCD( *( Data + 2 ) );

    if( Hours >= 0 && Hours < 24 && Minutes >= 0 && Minutes < 60) {
        DataStorage.tm.tm_hour = Hours;
        DataStorage.tm.tm_min = Minutes;
        Flag = 1;
    }
    else {
        Flag = 0;
    }

    return Flag;
}

static void CanTp_SingleFrameTx( uint8_t *Data, uint8_t Size ) {
    uint8_t MessageOutput[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    for( uint8_t i = 0; i < Size; i++ ) {
        MessageOutput[0]++;
        if( i > 0 ) {
            MessageOutput[i] = *( Data );
        }
    }

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, MessageOutput );
}

static uint8_t CanTp_SingleFrameRx( uint8_t *Data, uint8_t *Size ) {
    uint8_t CeroCounter = 0;
    uint8_t Flag;

    if( Message ) {
        Message = 0;

        *( Size ) = RxData[0];

        for( uint8_t i = 0; i < 7; i++) {
            *( Data + i ) = RxData[i + 1];
            if ( RxData[i + 1] == 0) {
                CeroCounter++;
            }
        }

        if( *( Size ) < 1 || *( Size ) > 8 ) {
            Flag = 0;
        }
        else if( CeroCounter == 7 ) {
            Flag = 0;
        }
        else {
            Flag = 1;
        }
    }
    else {
        Flag = 0;
    }
    
    CeroCounter = 0;

    return Flag;
}