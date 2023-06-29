#include "app_serial.h"

#define IDLE    1
#define MESSAGE 2
#define TIME    3
#define DATE    4
#define ALARM   5
#define ERROR   6
#define OK      7

#define JAN ( uint8_t ) 1
#define FEB ( uint8_t ) 2
#define MAR ( uint8_t ) 3
#define APR ( uint8_t ) 4
#define MAY ( uint8_t ) 5
#define JUN ( uint8_t ) 6
#define JUL ( uint8_t ) 7
#define AUG ( uint8_t ) 8
#define SEP ( uint8_t ) 9
#define OCT ( uint8_t ) 10
#define NOV ( uint8_t ) 11
#define DEC ( uint8_t ) 12

extern void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs );
uint8_t HexToBCD(uint8_t Data);
static uint8_t TimeValidaton( uint8_t *Data );
static uint8_t DateValidaton( uint8_t *Data );
static uint8_t WeekDay( uint8_t *Data );
uint16_t YearDay(uint8_t *Data );
static uint8_t DaylightSavingTime( uint8_t *Data );
static uint8_t AlarmValidaton( uint8_t *Data );
static void CanTp_SingleFrameTx( uint8_t *Data, uint8_t Size );
static uint8_t CanTp_SingleFrameRx( uint8_t *Data, uint8_t *Size );

extern FDCAN_RxHeaderTypeDef   CANRxHeader;
extern FDCAN_TxHeaderTypeDef   CANTxHeader;
extern FDCAN_FilterTypeDef     CANFilter;

FDCAN_HandleTypeDef     CANHandler  = {0};
FDCAN_RxHeaderTypeDef   CANRxHeader = {0};
FDCAN_TxHeaderTypeDef   CANTxHeader = {0};
FDCAN_FilterTypeDef     CANFilter   = {0};

APP_MsgTypeDef  DataStorage = {0};
APP_Messages    MessageType;

extern uint8_t RxData[8];
extern uint8_t MessageData[7];
extern uint8_t MessageSize;
extern uint8_t Message;

uint8_t RxData[8]       = {0};
uint8_t MessageData[7]  = {0};
uint8_t MessageSize     = 0;
uint8_t Message         = 0;

extern uint8_t State;

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
    uint8_t MessageOK    = ( uint8_t ) 0x55;
    uint8_t MessageERROR = ( uint8_t ) 0xAA;

    switch( State ) {
        case IDLE:
            if( CanTp_SingleFrameRx( MessageData, &MessageSize ) == ( uint8_t ) 1 ) {
                State = MESSAGE;
            }
        break;

        case MESSAGE:
            //printf("Entramos a MESSAGE.\n\r");
            if( MessageData[0] == ( uint8_t ) SERIAL_MSG_TIME ) {
                DataStorage.msg = SERIAL_MSG_TIME;
                State = TIME;
            }
            else if( MessageData[0] == ( uint8_t ) SERIAL_MSG_DATE ) {
                DataStorage.msg = SERIAL_MSG_DATE;
                State = DATE;
            }
            else if( MessageData[0] == ( uint8_t ) SERIAL_MSG_ALARM ) {
                DataStorage.msg = SERIAL_MSG_ALARM;
                State = ALARM;
            }
            else {
                State = ERROR;
            }
        break;

        case TIME:
            //printf("Vas a configurar hora.\n\r");
            if( TimeValidaton( MessageData ) == ( uint8_t ) 1 ) {
                //printf("Hora: %u\n\r", ( unsigned int ) DataStorage.tm.tm_hour );
                //printf("Minutos: %u\n\r", ( unsigned int ) DataStorage.tm.tm_min );
                //printf("Segundos: %u\n\r", ( unsigned int ) DataStorage.tm.tm_sec );
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case DATE:
            //printf("Vas a configurar fecha.\n\r");
            if( DateValidaton( MessageData ) == ( uint8_t ) 1 ) {
                //printf("Dia: %u\n\r", ( unsigned int ) DataStorage.tm.tm_mday );
                //printf("Mes: %u\n\r", ( unsigned int ) DataStorage.tm.tm_mon );
                //printf("Anio: %u\n\r", ( unsigned int ) DataStorage.tm.tm_year );
                DataStorage.tm.tm_wday = WeekDay( MessageData );
                //printf("Weekday: %u\n\r", ( unsigned int ) DataStorage.tm.tm_wday );
                DataStorage.tm.tm_yday = YearDay( MessageData );
                //printf("Yearday: %u\n\r", ( unsigned int ) DataStorage.tm.tm_yday );
                DataStorage.tm.tm_isdst = DaylightSavingTime( MessageData );
                //printf("Daylight Saving Time: %u\n\r", ( unsigned int ) DataStorage.tm.tm_isdst );
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ALARM:
            //printf("Vas a configurar alarma.\n\r");
            if( AlarmValidaton( MessageData ) == ( uint8_t ) 1) {
                //printf("Hora de alarma: %u\n\r", ( unsigned int ) DataStorage.tm.tm_hour_a );
                //printf("Minutos de alarma: %u\n\r", ( unsigned int ) DataStorage.tm.tm_min_a );
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ERROR:
            //printf("ERROR.\n\r");
            //DataStorage.msg = MessageERROR;
            //CanTp_SingleFrameTx( &DataStorage.msg, 2 );
            CanTp_SingleFrameTx( &MessageERROR, 2 );
            State = IDLE;
        break;

        case OK:
            //printf("OK.\n\r");
            //DataStorage.msg = MessageOK;
            //CanTp_SingleFrameTx( &DataStorage.msg, 2 );
            CanTp_SingleFrameTx( &MessageOK, 2 );
            State = IDLE;
        break;

        default:
        break;
    }
}

/* cppcheck-suppress misra-c2012-2.7 ; Function defined by the HAL library. */
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
  HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );
  Message = 1;
}

uint8_t HexToBCD( uint8_t Data ) {
    uint8_t DataBCD;

    DataBCD = ( ( ( Data >> ( uint8_t ) 4 ) * ( uint8_t ) 10 ) + ( Data & ( uint8_t ) 0x0F ) );

    return DataBCD;
}

static uint8_t TimeValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Hours = HexToBCD( Data[1] );
    uint8_t Minutes = HexToBCD( Data[2] );
    uint8_t Seconds = HexToBCD( Data[3] );

    if( ( Hours >= ( uint8_t ) 0 ) && ( Hours < ( uint8_t ) 24 ) && ( Minutes >= ( uint8_t ) 0 ) &&  ( Minutes < ( uint8_t ) 60 ) && ( Seconds >= ( uint8_t ) 0 ) && ( Seconds <  ( uint8_t ) 60 ) ) {
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

static uint8_t DateValidaton( uint8_t *Data ) {
    uint8_t Flag = 0;
    uint8_t Day = HexToBCD( Data[1] );
    uint8_t Month = HexToBCD( Data[2] );
    uint16_t Year = ( ( uint16_t ) HexToBCD( Data[3] ) * ( uint16_t ) 100 ) + ( uint16_t ) HexToBCD( Data[4] );

    if( ( Year > ( uint16_t ) 1900 ) && ( Year < ( uint16_t ) 2100 ) ) {
        DataStorage.tm.tm_year = Year;

        if( ( Month >= JAN ) && ( Month <= DEC ) ) {
            DataStorage.tm.tm_mon = Month;

            if( ( Month == JAN ) || ( Month == MAR ) || ( Month == MAY ) || ( Month == JUL )  || ( Month == AUG ) || ( Month == OCT ) || ( Month == DEC ) ) {
                if( ( Day >= ( uint8_t ) 1 ) &&  ( Day <= ( uint8_t ) 31 ) ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else if( ( Month == APR ) || ( Month == JUN ) || ( Month == SEP ) ||  ( Month == NOV ) ) {
                if( ( Day >= ( uint8_t ) 1 ) && ( Day <= ( uint8_t ) 30 ) ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else if( ( ( ( Year % ( uint16_t ) 4 ) ) == ( uint16_t ) 0 ) && ( Month == ( uint16_t ) FEB ) ) {
                if( ( Day >= ( uint8_t ) 1 ) && ( Day <= ( uint8_t ) 29 ) ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else if ( Month == FEB ) {
                if( ( Day >= ( uint8_t ) 1 ) && ( Day <= ( uint8_t ) 28 ) ) {
                    DataStorage.tm.tm_mday = Day;
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }
            else {
                Flag = 0;
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

static uint8_t WeekDay( uint8_t *Data ) {
    uint8_t Day = HexToBCD( Data[1] );
    uint8_t Month = HexToBCD( Data[2] );
    uint16_t Year = ( ( uint16_t ) HexToBCD( Data[3] ) * ( uint16_t ) 100 ) + ( uint16_t ) HexToBCD( Data[4] );
    uint16_t k;
    uint16_t j;
    uint16_t h;

    if (Month == JAN) {
        Month = 13;
        Year--;
    }
    if (Month == FEB) {
        Month = 14;
        Year--;
    }
    k = Year % ( uint16_t ) 100;
    j = Year / ( uint16_t ) 100;
    h = ( ( ( uint16_t ) Day + ( uint16_t ) 3 ) * ( ( uint16_t ) Month + ( uint16_t ) 1 ) / ( ( uint16_t ) 5 + k + k ) / ( ( uint16_t ) 4 + j ) / ( ( uint16_t ) 4 + ( uint16_t ) 5 ) * j ) % ( uint16_t ) 7;

    return h;
}

uint16_t YearDay(uint8_t *Data ) {
    uint8_t Day = HexToBCD( Data[1] );
    uint8_t Month = HexToBCD( Data[2] );
    uint16_t Year = ( ( uint16_t ) HexToBCD( Data[3] ) * ( uint16_t ) 100 ) + ( uint16_t ) HexToBCD( Data[4] );
    uint16_t Result;

    switch( Month ) {
        case JAN:
            Result = Day;
        break;

        case FEB:
            Result = Day + ( uint16_t ) 31;
        break;

        case MAR:
            Result = Day + ( uint16_t ) 59;
        break;

        case APR:
            Result = Day + ( uint16_t ) 90;
        break;

        case MAY:
            Result = Day + ( uint16_t ) 120;
        break;

        case JUN:
            Result = Day + ( uint16_t ) 151;
        break;

        case JUL:
            Result = Day + ( uint16_t ) 181;
        break;

        case AUG:
            Result = Day + ( uint16_t ) 212;
        break;

        case SEP:
            Result = Day + ( uint16_t ) 243;
        break;

        case OCT:
            Result = Day + ( uint16_t ) 273;
        break;

        case NOV:
            Result = Day + ( uint16_t ) 304;
        break;

        case DEC:
            Result = Day + ( uint16_t ) 334;
        break;

        default:
        break;
    }

    if( ( ( Year % ( uint16_t ) 4 ) == ( uint16_t ) 0 ) && ( Month > FEB ) ) {
        Result += ( uint16_t ) 1;
    }

    return Result;
}

static uint8_t DaylightSavingTime( uint8_t *Data ) {
    uint8_t Flag;
    uint16_t Year = ( ( uint16_t ) HexToBCD( Data[3] ) * ( uint16_t ) 100 ) + ( uint16_t ) HexToBCD( Data[4] );
    uint16_t DayNumber = YearDay( Data );
    uint16_t Start = 71; //From March 12th
    uint16_t End = 309; //to November 5th.

    if( ( Year % ( uint16_t ) 4 ) == ( uint16_t ) 0 ) {
        Start++;
        End++;
    }

    if( ( DayNumber >= Start ) && ( DayNumber <= End ) ) {
        Flag = 1;
    }
    else {
        Flag = 0;
    }

    return Flag;
}

static uint8_t AlarmValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Hours = HexToBCD( Data[1] );
    uint8_t Minutes = HexToBCD( Data[2] );

    if( ( Hours >= ( uint16_t ) 0 ) && ( Hours < ( uint16_t ) 24 ) && ( Minutes >= ( uint16_t ) 0 ) && ( Minutes < ( uint16_t ) 60 ) ) {
        DataStorage.tm.tm_hour_a = Hours;
        DataStorage.tm.tm_min_a = Minutes;
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
        if( i > ( uint8_t ) 0 ) {
            MessageOutput[i] = *( Data );
        }
    }

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, MessageOutput );
}

static uint8_t CanTp_SingleFrameRx( uint8_t *Data, uint8_t *Size ) {
    uint8_t CeroCounter = 0;
    uint8_t Flag;

    if( Message == ( uint8_t ) 1 ) {
        Message = 0;

        *( Size ) = RxData[0];

        for( uint8_t i = 0; i < ( uint8_t ) 7; i++) {
            Data[i] = RxData[i + ( uint8_t ) 1];
            if ( RxData[i + ( uint8_t ) 1] == ( uint8_t ) 0) {
                CeroCounter++;
            }
        }

        if( ( *( Size ) < 1 ) || ( *( Size ) > 8 ) ) {
            Flag = 0;
        }
        else if( CeroCounter == ( uint8_t ) 7 ) {
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