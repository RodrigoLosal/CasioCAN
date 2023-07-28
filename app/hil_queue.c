/**
 * @file    hil_queue.c
 * @brief   ****
 *
 * Interfaces for LCD driver:
 * 
 * uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd ) : Use once to Initialize the lcd
 * 
 * 
 */

#include "hil_queue.h"

/**
 * @brief   Initializes the queue by setting the head and tail elements to zero, and the values of empty to one and full to zero
 * @param   hqueue Queue Handler
 */
void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue )
{
    hqueue->Head          = 0;
    hqueue->Tail          = 0;
    hqueue->Full          = 0;
    hqueue->Empty         = 1;
    hqueue->SavedElements = 0;
}


/**
 * @brief   Copies the information referenced by the empty pointer data to the buffer controlled by hqueue
 * @param   hqueue Queue Handler
 * @param   data Variable where data will be copied
 */
uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t valueToReturn;

    if( HIL_QUEUE_IsFull( hqueue ) == ( uint8_t ) 1 ){
        hqueue->Full = 1;
        valueToReturn = QUEUE_NOT_OK;
    }

    else{
        ( void ) memcpy(hqueue->Buffer + (hqueue->Size * hqueue->Tail), data, hqueue->Size);
        hqueue->Tail = (hqueue->Tail + ( uint32_t ) 1) % hqueue->Elements;
        hqueue->SavedElements++;
        
        valueToReturn = QUEUE_OK;

        if(hqueue->Tail == hqueue->Elements){
            hqueue->Tail = 0;
        }
    }
    
    return valueToReturn;
}


/**
 * @brief   Reads data from the buffer controlled by hqueue, the data is copied into the data type referenced by the empty pointer data
 * @param   hqueue Queue Handler
 * @param   data Variable where data will be copied
 */
uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t valueToReturn;

    if( HIL_QUEUE_IsEmpty( hqueue ) == ( uint8_t ) 1 ){
        hqueue->Empty = 1;
        valueToReturn = QUEUE_NOT_OK;
    }

    else{
        ( void ) memcpy(data, hqueue->Buffer + (hqueue->Size * hqueue->Head), hqueue->Size);
        hqueue->Head = ( hqueue->Head + ( uint32_t ) 1 ) % hqueue->Elements;
        hqueue->SavedElements--;

        valueToReturn = QUEUE_OK;

        if(hqueue->Head == hqueue->Elements){
            hqueue->Head = 0;
        }
    }

    return valueToReturn;
}


/**
 * @brief   The function returns a one if there are no more elements that can be read from the queue and zero if there is at least one element that can be read.
 * @param   hqueue Queue handler
 */
uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue )
{
    return (hqueue->SavedElements == ( uint32_t ) 0);
}


/**
 * @brief   The function must empty the queue in case it has elements inside it, the information will be discarded
 * @param   hqueue Queue handler 
 */
void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue )
{
    hqueue->Head          = 0;
    hqueue->Tail          = 0;
    hqueue->Full          = 0;
    hqueue->Empty         = 1;
    hqueue->SavedElements = 0;
}


/**
 * @brief   The function returns a one if the queue is full, 0 otherwise
 * @param   hqueue Queue handler
 */
uint8_t HIL_QUEUE_IsFull( QUEUE_HandleTypeDef *hqueue )
{
    return (hqueue->SavedElements == hqueue->Elements);
}

/**
 * @brief   Copies the information referenced by the empty pointer data to the buffer controlled by hqueue and disable interrupt
 * @param   hqueue Queue Handler
 * @param   data Variable where data will be copied
 * @param   isr Interrupt ID that will be disabled
 */
uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    uint8_t valueToReturn;

    if( isr == ( uint8_t ) 0xFF )                       //Enable/disable all interrupt
    {
        __disable_irq();
        valueToReturn = HIL_QUEUE_Write( hqueue, data );
        __enable_irq();
    }

    else
    {
        if( ( isr >= ( uint8_t ) 0 ) && ( isr <= ( uint8_t ) 30) )          //Validate that isr is a member of IRQn_Type enum
        {  
            HAL_NVIC_DisableIRQ( isr );
            valueToReturn = HIL_QUEUE_Write( hqueue, data );
            HAL_NVIC_EnableIRQ( isr );
        }
        else
        {
            valueToReturn = QUEUE_NOT_OK;
        }
    }
    
    return valueToReturn;
}

/**
 * @brief   Reads data from the buffer controlled by hqueue, the data is copied into the data type referenced by the empty pointer data  and operates with interrupt
 * @param   hqueue Queue Handler
 * @param   data Variable where data will be copied
 * @param   isr Interrupt ID that will be disabled
 */
uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    uint8_t valueToReturn;

    if( isr == ( uint8_t ) 0xFF )                       //Enable/disable all interrupt
    {
        __disable_irq();
        valueToReturn = HIL_QUEUE_Read( hqueue, data );
        __enable_irq();
    }

    else
    {
        if( ( isr >= ( uint8_t ) 0 ) && ( isr <= ( uint8_t ) 30 ) )          //Validate that isr is a member of IRQn_Type enum
        {  
            HAL_NVIC_DisableIRQ( isr );
            valueToReturn = HIL_QUEUE_Read( hqueue, data );
            HAL_NVIC_EnableIRQ( isr );
        }
        else
        {
            valueToReturn = QUEUE_NOT_OK;
        }
    }
    
    return valueToReturn;
}


/**
 * @brief   The function returns a one if there are no more elements that can be read from the queue and zero if there is at least one element that can be read.
 * @param   hqueue Queue Handler
 * @param   isr Interrupt ID that will be disabled
 */
uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{
    uint8_t valueToReturn;

    if( isr == ( uint8_t ) 0xFF )                       //Enable/disable all interrupt
    {
        __disable_irq();
        valueToReturn = HIL_QUEUE_IsEmpty( hqueue );
        __enable_irq();
    }

    else
    {
        if( ( isr >= ( uint8_t ) 0 ) && ( isr <= ( uint8_t ) 30 ) )          //Validate that isr is a member of IRQn_Type enum
        {  
            HAL_NVIC_DisableIRQ( isr );
            valueToReturn = HIL_QUEUE_IsEmpty( hqueue );
            HAL_NVIC_EnableIRQ( isr );
        }
        else
        {
            valueToReturn = QUEUE_NOT_OK;
        }
    }
    
    return valueToReturn;
}

/**
 * @brief   The function must empty the queue in case it has elements inside it, the information will be discarded and operates wih interrupt
 */
void HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{
    if( isr == ( uint8_t ) 0xFF )                       //Enable/disable all interrupt
    {
        __disable_irq();
        HIL_QUEUE_Flush( hqueue );
        __enable_irq();
    }

    else
    {
        if( ( isr >= ( uint8_t ) 0 ) && ( isr <= ( uint8_t ) 30 ) )          //Validate that isr is a member of IRQn_Type enum
        {  
            HAL_NVIC_DisableIRQ( isr );
            HIL_QUEUE_Flush( hqueue );
            HAL_NVIC_EnableIRQ( isr );
        }
        else{
        }
    }
    
}