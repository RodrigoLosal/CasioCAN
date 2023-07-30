/**
 * @file    app_serial.c
 * @brief   **File that manages the functioning of the CAN transmition & reception.**
 *
 * In this file, the state machine that manages what is done with the messages that 
 * the user sends through the CAN Bus is initialized and executed. It is validated if the data received 
 * is correct, and if so, it will be used to send it. to a structure using the app_clock.c file.
 */

#include "app_serial.h"

/** 
  * @defgroup <SerialStates>
  * 
  @{ */
#define IDLE        1 /*!< First state of the serial state machine.*/
#define RECEPTION   2 /*!< Second state of the serial state machine.*/
#define MESSAGE     3 /*!< Third state of the serial state machine.*/
#define TIME        4 /*!< Fourth state of the serial state machine.*/
#define DATE        5 /*!< Fifth state of the serial state machine.*/
#define ALARM       6 /*!< Sixth state of the serial state machine.*/
#define ERROR       7 /*!< Seventh state of the serial state machine.*/
#define OK          8 /*!< Eighth state of the serial state machine.*/
/**
  @} */

/** 
  * @defgroup <Months>
  @{ */
#define JAN ( uint8_t ) 1 /*!< January.*/
#define FEB ( uint8_t ) 2 /*!< February.*/
#define MAR ( uint8_t ) 3 /*!< March.*/
#define APR ( uint8_t ) 4 /*!< April.*/
#define MAY ( uint8_t ) 5 /*!< May.*/
#define JUN ( uint8_t ) 6 /*!< June.*/
#define JUL ( uint8_t ) 7 /*!< July.*/
#define AUG ( uint8_t ) 8 /*!< August.*/
#define SEP ( uint8_t ) 9 /*!< September.*/
#define OCT ( uint8_t ) 10 /*!< October.*/
#define NOV ( uint8_t ) 11 /*!< November.*/
#define DEC ( uint8_t ) 12 /*!< December.*/
/**
  @} */

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
static uint32_t Serial_Machine(uint32_t currentState);

/**
  * @brief   Structure that will contain the values to initialice the CAN module.
  */
FDCAN_HandleTypeDef     CANHandler  = {0};

/**
  * @brief   Structure that will contain the values to initialice the CAN Rx module.
  */

extern FDCAN_RxHeaderTypeDef   CANRxHeader;
FDCAN_RxHeaderTypeDef   CANRxHeader = {0};

/**
  * @brief   Structure that will contain the values to initialice the CAN Tx module.
  */

extern FDCAN_TxHeaderTypeDef   CANTxHeader;
FDCAN_TxHeaderTypeDef   CANTxHeader = {0};

/**
  * @brief   Structure that will contain the values to initialice the CAN filter module.
  */

extern FDCAN_FilterTypeDef     CANFilter;
FDCAN_FilterTypeDef     CANFilter   = {0};

/**
  * @brief   Structure that contains the type of message received and it's values.
  */

APP_MsgTypeDef  DataStorage = {0};

/**
  * @brief   Enum that defines which type of message is received by the CAN bus.
  */

APP_Messages    MessageType;

/**
 * @brief  Variable that will contain the data received in the Fifo0 buffer.
 */

extern uint8_t RxData[8];
uint8_t RxData[8]       = {0};


/**
 * @brief  Variable that will contain the unpacked CAN message.
 */

extern uint8_t MessageData[7];
uint8_t MessageData[7]  = {0};

/**
 * @brief  Variable that will contain the size number of the received CAN message.
 */

extern uint8_t MessageSize;
uint8_t MessageSize     = 0;

/**
 * @brief  Variable that will contain the flag the flag that warns if there is a new message.
 */

extern uint8_t Message;
uint8_t Message         = 0;

/**
 * @brief  Variable for the change of the cases of the switch of the state machine.
 */

//extern uint32_t State;
//uint32_t State = IDLE;

/**
 * @brief Struct variable of Queue elements
*/
extern QUEUE_HandleTypeDef CanQueue;
QUEUE_HandleTypeDef CanQueue = {0};

/**
 * @brief Struct variable with the array of Queue
*/
/* cppcheck-suppress misra-c2012-8.7 ;If header is modified the program will not work*/
NEW_MsgTypeDef buffer_serial[9];  /* cppcheck-suppress misra-c2012-8.4 ;Its been used due to the queue*/

/**
 * @brief Struct variable with array to save the elements
*/
extern NEW_MsgTypeDef RxBuffer;
NEW_MsgTypeDef RxBuffer = {0};

/**
 * @brief   **Function that initialices the registers of the CAN communication protocol.**
 *
 * CAN frame is configured as Classic, the transmition identifier is set witht the value 0x122, and the
 * filter is set to accept only messages with the identifier 0x111.
 */

void Serial_Init( void )
{
    CanQueue.Buffer = (void*)buffer_serial;     /*Indicate the buffer that the tail will use as memory space*/
    CanQueue.Elements = QUEUE_ELEMENTS;         /*Indicates the maximum number of elements that can be stored*/ 
    CanQueue.Size = sizeof( NEW_MsgTypeDef );   /*Indicate the size in bytes of the type of elements to handle*/ 
    HIL_QUEUE_Init( &CanQueue );                /*Initialize the queue*/ 

    HAL_StatusTypeDef Status;

    CANHandler.Instance                     = FDCAN1;
    CANHandler.Init.Mode                    = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat             = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider            = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode         = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.NominalPrescaler        = 20;
    CANHandler.Init.NominalSyncJumpWidth    = 1;
    CANHandler.Init.NominalTimeSeg1         = 11;
    CANHandler.Init.NominalTimeSeg2         = 4;
    CANHandler.Init.StdFiltersNbr           = 1;

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_Init( &CANHandler);
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );

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

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_ConfigGlobalFilter( &CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_Start( &CANHandler);
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );
}

void Serial_Task(void) {
   static uint32_t state = RECEPTION;
   static uint32_t serialtick =0;
   /* We check the waiting queue with 10ms */
   if ((HAL_GetTick() - serialtick) >= 10u) {
       serialtick = HAL_GetTick(); 
       
        state = Serial_Machine(state);
   }
}

/**
 * @brief   **Functon with the state machine that manages the CAN inputs & outputs.**
 *
 * The state machine calls functions to unpack the data received and to validate it's congruence,
 * afterwards it sends an OK or ERROR message. More details are shown in the diagram.
 *
 */

static uint32_t Serial_Machine( uint32_t currentState )
{
    uint8_t MessageOK    = ( uint8_t ) 0x55;
    uint8_t MessageERROR = ( uint8_t ) 0xAA;
    uint32_t State;

    State = currentState;

    switch( State ) {
        case IDLE:
            /*if( CanTp_SingleFrameRx( MessageData, &MessageSize ) == ( uint8_t ) 1 ) {
                State = MESSAGE;
            }*/
            State = RECEPTION;
        break;

        case RECEPTION:
            /*Revision and unpaked the messages */
           if( HIL_QUEUE_IsEmptyISR( &CanQueue, TIM16_FDCAN_IT0_IRQn ) == ( uint8_t ) 0 )
            {
                /*Read the first message*/
                (void)HIL_QUEUE_ReadISR( &CanQueue, &RxBuffer, TIM16_FDCAN_IT0_IRQn );

                /*Revision and unpaked the messages */
                if( CanTp_SingleFrameRx( RxBuffer.data, &MessageSize) == ( uint8_t ) 1 )
                {
                    State = MESSAGE;
                }
            }else{
                
                State = IDLE;
            }
        break;

        case MESSAGE:
            if( RxBuffer.data[0] == ( uint8_t ) SERIAL_MSG_TIME ) {
                DataStorage.msg = SERIAL_MSG_TIME;
                State = TIME;
            }
            else if( RxBuffer.data[0] == ( uint8_t ) SERIAL_MSG_DATE ) {
                DataStorage.msg = SERIAL_MSG_DATE;
                State = DATE;
            }
            else if( RxBuffer.data[0] == ( uint8_t ) SERIAL_MSG_ALARM ) {
                DataStorage.msg = SERIAL_MSG_ALARM;
                State = ALARM;
            }
            else {
                State = ERROR;
            }
        break;

        case TIME:
            if( TimeValidaton( RxBuffer.data ) == ( uint8_t ) 1 ) {

                (void) HIL_QUEUE_WriteISR( &ClockQueue, &DataStorage, 0xFF );
                
                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case DATE:
            if( DateValidaton( RxBuffer.data ) == ( uint8_t ) 1 ) {

                DataStorage.tm.tm_wday = WeekDay( RxBuffer.data );
                DataStorage.tm.tm_yday = YearDay( RxBuffer.data );
                DataStorage.tm.tm_isdst = DaylightSavingTime( RxBuffer.data );

                (void) HIL_QUEUE_WriteISR( &ClockQueue, &DataStorage, 0xFF );

                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ALARM:
            if( AlarmValidaton( RxBuffer.data ) == ( uint8_t ) 1) {

                (void)HIL_QUEUE_WriteISR( &ClockQueue, &DataStorage, 0xFF );

                State = OK;
            }
            else {
                State = ERROR;
            }
        break;

        case ERROR:
            CanTp_SingleFrameTx( &MessageERROR, 2 );
            State = IDLE;
        break;

        case OK:
            CanTp_SingleFrameTx( &MessageOK, 2 );
            State = IDLE;
        break;

        default:
        break;
    }

    return State;
}

/**
 * @brief   **Function triggered by the Rx interruption to read the Fifo0 buffer.**
 *
 * A flag is set whenever the interruption happens, and reset in the function that unpacks the
 * message.
 */

/* cppcheck-suppress misra-c2012-2.7 ; Function defined by the HAL library. */
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs ){
    HAL_StatusTypeDef Status;

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );

    (void) HIL_QUEUE_WriteISR( &CanQueue, &RxData, TIM16_FDCAN_IT0_IRQn );

    Message = 1;
}

/**
 * @brief   **Function that transforms hex data to BCD format.**
 *
 * @retval  The transformed number to BCD format is returned.
 *
 * @note This function supports only 8 bit inputs.
 */

uint8_t HexToBCD( uint8_t Data ) {
    uint8_t DataBCD;

    DataBCD = ( ( ( Data >> ( uint8_t ) 4 ) * ( uint8_t ) 10 ) + ( Data & ( uint8_t ) 0x0F ) );

    return DataBCD;
}

/**
 * @brief   **Function to validate if the received time format is correct.**
 *
 * The hour must be between 0 & 24, the minutes between 0 & 59, the seconds between 0 & 59.
 *
 * @param   <*Data[in]> This pointer has the address of the unpacked CAN message.
 *
 * @retval  A flag that lets the state machine know if the data passed the validation. 1 is for true
 * and 0 is for false.
 * 
 */

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

/**
 * @brief   **Function to validate if the received date is correct.**
 *
 * The year must be between 1900 & 2100, the month between 1 & 12, and the day depends on the month.
 *
 * @param   <*Data[in]> This pointer is expected to have the address of the unpacked CAN message.
 *
 * @retval  A flag that lets the state machine know if the data passed the validation. 1 is for true
 * and 0 is for false.
 * 
 * @note This function takes into consideraton leap years.
 */

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

/**
 * @brief   **Function to calculate which day of the week is a given date.**
 *
 * This function utilices the Zeller's congruence formula.
 *
 * @param   <*Data[in]> This pointer has the address of the unpacked CAN message.
 *
 * @retval  The function returns the variable "h", which is the day of the week 
 * (0 = Saturday, 1 = Sunday, 2 = Monday, …, 6 = Friday).
 */

static uint8_t WeekDay( uint8_t *Data ) {
    uint8_t Day = HexToBCD( Data[1] );
    uint8_t Month = HexToBCD( Data[2] );
    uint16_t Year = ( ( uint16_t ) HexToBCD( Data[3] ) * ( uint16_t ) 100 ) + ( uint16_t ) HexToBCD( Data[4] );
    
    /*uint16_t k;
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

    return h;*/

    //Zeller algorithm
    uint8_t aux = ((uint8_t)14-Month) / (uint8_t)12;
    uint16_t yearZeller= Year-aux;
    uint8_t monthZeller= Month + (uint8_t)12*aux - (uint8_t)2;
     
    uint8_t diaSemana = 0;
    
    if(Day <= (uint8_t)31){
        if (Month <= (uint8_t)12)
        {
            if(Month == (uint8_t)2){
                if(((Year % (uint16_t)4 == (uint16_t)0) && (Year % (uint16_t)100 != (uint16_t)0)) || (Year % (uint16_t)400 == (uint16_t)0)){//revision si el año es bisiesto
                    diaSemana = (uint8_t)((Day + yearZeller + (yearZeller / (uint16_t)4) - (yearZeller / (uint16_t)100) + (yearZeller / (uint16_t)400) + ((uint16_t)31 * monthZeller) / (uint16_t)12) % (uint8_t)7);
            }
            }else if((Month == (uint8_t)1) || (Month == (uint8_t)3) || (Month == (uint8_t)5) || (Month == (uint8_t)7) || (Month == (uint8_t)8) || (Month == (uint8_t)10) || (Month == (uint8_t)12)){
                if(Day <= (uint8_t)31){
                    diaSemana = (uint8_t)((Day + yearZeller + (yearZeller / (uint16_t)4) - (yearZeller / (uint16_t)100) + (yearZeller / (uint16_t)400) + ((uint16_t)31 * monthZeller) / (uint16_t)12) % (uint8_t)7);
                }
                
            }else if((Month == (uint8_t)4) || (Month == (uint8_t)6) || (Month == (uint8_t)9) || (Month == (uint8_t)11)){
                diaSemana = (uint8_t)((Day + yearZeller + (yearZeller / (uint16_t)4) - (yearZeller / (uint16_t)100) + (yearZeller / (uint16_t)400) + ((uint16_t)31 * monthZeller) / (uint16_t)12) % (uint8_t)7);
            }else{
                diaSemana= 7;
                
            } 
        }else{
            diaSemana= 7;
        }
               
    }else{
        diaSemana= 7;
    }
   
   return diaSemana;
}

/**
 * @brief   **Function to calculate the day # of the year.**
 *
 * @param   <*Data> This pointer has the address of the unpacked CAN message.
 *
 * @retval  The variable "result" goes from 1 to 365 (or 366).
 *
 * @note This function takes into consideration leap years.
 */

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

/**
 * @brief   **Function to calculate if a given date is within the daylight saving time season.**
 *
 * The daylight saving time season goes from march 12th to november 5th, 
 *
 * @param   <*Data[in]> This pointer has the address of the unpacked CAN message.
 *
 * @retval  A flag whose values are 1 if the given date matches the daylight saving time
 * season, and 0 if it doesn't.
 *
 * @note Even though the daylight saving time season dates change each year, it was decided to keep 
 * the dates of the function constant because there is no predictable pattern that defines the dates 
 * of each subsequent year.
 */

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

/**
 * @brief   **Function to validate if the received alarm format is correct.**
 *
 * The hour must be between 0 & 24, the minutes between 0 & 59.
 *
 * @param   <*Data[in]> This pointer has the address of the unpacked CAN message.
 *
 * @retval  A flag that lets the state machine know if the data passed the validation. 1 is for true
 * and 0 is for false.
 * 
 * @note Seconds are not taken into account, since they aren't necessary.
 * 
 */

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

/**
 * @brief   **Function to pack and send the OK or ERROR message.**
 * 
 * The message is packed into an 8 bit - 8 element array and sent to the Tx Fifo buffer afterwards.
 *
 * @param   <*Data[in]> This pointer has the address of the OK or ERROR message.
 * @param   <Size[in]> This variable must have the size # of the message that the user wants to send.
 */

static void CanTp_SingleFrameTx( uint8_t *Data, uint8_t Size ) {
    HAL_StatusTypeDef Status;
    uint8_t MessageOutput[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    for( uint8_t i = 0; i < Size; i++ ) {
        MessageOutput[0]++;
        if( i > ( uint8_t ) 0 ) {
            MessageOutput[i] = *( Data );
        }
    }

    /*The function is used and its result is verified.*/
    Status = HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, MessageOutput );
    /*cppcheck-suppress misra-c2012-11.8 ; Macro required for functional safety.*/
    assert_error( Status == HAL_OK, CAN_RET_ERROR );
}

/**
 * @brief   **Function to unpack the message recieved into the Fifo0 buffer.**
 *
 * This function saves the first element of the received string to define the size of the rest of the 
 * message, and in function of this number, saves into a new global array the rest of the elements.
 *
 * @param   <*Data[in]> Pointer to the adress of the variable that will contain the unpacked message.
 * @param   <*Size[in]> Pointer to the adress of the variable that will contain the single frame message size.
 * @param   <*Data[out]> Pointer to the adress of the variable that will contain the unpacked message.
 * @param   <*Size[out]> Pointer to the adress of the variable that will contain the single frame message size.
 *
 * @retval  A flag variable returns 1 if a certain number of bytes were received, otherwise a 0, 
 * when no message was received yet or the message is not complain with CAN-TP single frame format.
 */

static uint8_t CanTp_SingleFrameRx( uint8_t *Data, uint8_t *Size ) {
    uint8_t Flag;

    if( Message == ( uint8_t ) 1 ) {
        Message = 0;

        *( Size ) = RxData[0];

        for( uint8_t i = 0; i < ( uint8_t ) 7; i++) {
            Data[i] = RxData[i + ( uint8_t ) 1];
        }

        if( ( *( Size ) < 1 ) || ( *( Size ) > 8 ) ) {
            Flag = 0;
        }
        else {
            Flag = 1;
        }
    }
    else {
        Flag = 0;
    }

    return Flag;
}