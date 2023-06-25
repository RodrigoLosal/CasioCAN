#include "app_serial.h"
#include "app_bsp.h"
#include <stdio.h>

#define IDLE 1
#define MESSAGE 2
#define TIME 3
#define DATE  4
#define ALARM 5
#define ERROR 6
#define OK 7

void    HexToBCD(uint8_t *Data);
uint8_t TimeValidaton( uint8_t *Data );
uint8_t DateValidaton( uint8_t *Data );
uint8_t AlarmValidaton( uint8_t *Data );
static void CanTp_SingleFrameTx( uint8_t *Data, uint8_t Size );

FDCAN_HandleTypeDef     CANHandler;
FDCAN_RxHeaderTypeDef   CANRxHeader;
FDCAN_TxHeaderTypeDef   CANTxHeader;
FDCAN_FilterTypeDef     CANFilter;

uint8_t RxData[8];
uint8_t Message = 0;

uint8_t State = IDLE;

void Serial_Init( void )
{
    CANHandler.Instance                 = FDCAN1;
    CANHandler.Init.Mode                = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider        = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.NominalPrescaler    = 10;
    CANHandler.Init.NominalSyncJumpWidth = 1;
    CANHandler.Init.NominalTimeSeg1     = 11;
    CANHandler.Init.NominalTimeSeg2     = 4;
    CANHandler.Init.StdFiltersNbr       = 1;
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
            if( Message ) {
                Message = 0;
                State = MESSAGE;
            }
        break;

        case MESSAGE:
            printf("Entramos a MESSAGE.\n\r");
            if( RxData[0] == 0x08 && RxData[1] == 0x01 && RxData[6] == 0x00 && RxData[7] == 0x00) {
                State = TIME;
            }
            else if( RxData[0] == 0x08 && RxData[1] == 0x02 && RxData[6] == 0x00 && RxData[7] == 0x00) {
                State = DATE;
            }
            else if( RxData[0] == 0x08 && RxData[1] == 0x03 && RxData[6] == 0x00 && RxData[7] == 0x00) {
                State = ALARM;
            }
            else {
                State = ERROR;
            }
        break;

        case TIME:
            printf("Vas a configurar hora.\n\r");
            HexToBCD( RxData );
            if( TimeValidaton( RxData ) ) {
                printf("Hora: %u\n\r", RxData[2]);
                printf("Minutos: %u\n\r", RxData[3]);
                printf("Segundos: %u\n\r", RxData[4]);
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case DATE:
            printf("Vas a configurar fecha.\n\r");
            HexToBCD( RxData );
            if( DateValidaton( RxData ) ) {
                printf("Dia: %u\n\r", RxData[2]);
                printf("Mes: %u\n\r", RxData[3]);
                printf("Anio: %u\n\r", RxData[4]);
                printf("%u\n\r", RxData[5]);
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ALARM:
            printf("Vas a configurar alarma.\n\r");
            HexToBCD( RxData );
            if( AlarmValidaton( RxData ) ) {
                printf("Hora de alarma: %u\n\r", RxData[2]);
                printf("Minutos de alarma: %u\n\r", RxData[3]);
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ERROR:
            printf("ERROR.\n\r");
            CanTp_SingleFrameTx( &MessageERROR, 2 );
            State = IDLE;
        break;

        case OK:
            printf("OK.\n\r");
            CanTp_SingleFrameTx( &MessageOK, 2 );
            State = IDLE;
        break;
    }
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
  HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );
  Message = 1;
}

void HexToBCD( uint8_t *Data ) {
    for(uint8_t i = 0; i < 8; i++ ){
        Data[i] = ( ( ( Data[i] >> 4 ) * 10 ) + ( Data[i] & 0x0F ) );
    }
}

uint8_t TimeValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Hours = *( Data + 2 );
    uint8_t Minutes = *( Data + 3 );
    uint8_t Seconds = *( Data + 4 );

    if( Hours >= 0 && Hours < 24 && Minutes >= 0 && Minutes < 60 && Seconds >= 0 && Seconds < 60) {
        Flag = 1;
    }
    else {
        Flag = 0;
    }
    return Flag;
}

uint8_t DateValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Day = *( Data + 2 );
    uint8_t Month = *( Data + 3 );
    uint16_t Year = ( *( Data + 4 ) * 100 ) + *( Data + 5 );

    if( Year > 1900 && Year < 2100 ) {

        if( Month >= 1 && Month <= 12 ) {

            if( Month == 1 || Month == 3 || Month == 5 || Month == 7 || Month == 8 || Month == 10 || Month == 12 ) {
                if( Day >= 1 && Day <= 31 ) {
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            if( Month == 4 || Month == 6 || Month == 9 || Month == 11 ) {
                if( Day >= 1 && Day <= 30 ) {
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            if( ( Year % 4 ) == 0 && Month == 2 ) {
                if( Day >=1 && Day <= 29 ) {
                    Flag = 1;
                }
                else {
                    Flag = 0;
                }
            }

            else {
                if( Day >=1 && Day <=28 ) {
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

uint8_t AlarmValidaton( uint8_t *Data ) {
    uint8_t Flag;
    uint8_t Hours = *( Data + 2 );
    uint8_t Minutes = *( Data + 3 );

    if( *( Data + 4 ) == 0 && *( Data + 5 ) == 0 ) {
        if( Hours >= 0 && Hours < 24 && Minutes >= 0 && Minutes < 60) {
            Flag = 1;
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