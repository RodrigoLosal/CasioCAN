/**
 * @file    hil_queue.h
 * @brief   ****
 *
 * Interfaces for app_clock driver:
 * 
 * void Clock_Init( void ) : Use once to Initialize the app_clock. Don't requires any parameter
 * 
 * void Clock_Task( void ) : Run continuosuly. It allows to configure the clock 
 *
 */

#ifndef HIL_QUEUE_H
#define HIL_QUEUE_H

    #include "app_bsp.h"

    #define     QUEUE_OK            0
    #define     QUEUE_NOT_OK        1
    #define     QUEUE_ELEMENTS      9

    typedef struct
    {
        void        *Buffer;        /**< puntero al espacio de memoria usado como buffer por la cola        */
        uint32_t    Elements;       /**< numero de elementos a almacenar (tamano de la cola)                */
        uint8_t     Size;           /**< tamano del tipo de elementos a almacenar                           */
        uint32_t    Head;           /**< puntero que indica el siguiente espacio a escribir                 */
        uint32_t    Tail;           /**< puntero que indica el siguiente espacio a leer                     */
        uint8_t     Empty;          /**< bandera que indica si no hay elementos a leer                      */
        uint8_t     Full;           /**< bandera que indica si no se puede seguir escribiendo mas elementos */
        uint32_t    SavedElements;   /**< elementos guardados */
        //agregar más elementos si se requieren
    } QUEUE_HandleTypeDef;

    /**
     * @brief   Initializes the queue by setting the head and tail elements to zero, and the values of empty to one and full to zero
     */
    void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue );

    /**
     * @brief   Copies the information referenced by the empty pointer data to the buffer controlled by hqueue
     */
    uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data );

    /**
     * @brief   Reads data from the buffer controlled by hqueue, the data is copied into the data type referenced by the empty pointer data
     */
    uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data );

    /**
     * @brief   The function returns a one if there are no more elements that can be read from the queue and zero if there is at least one element that can be read.
     */
    uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue );

    /**
     * @brief   The function must empty the queue in case it has elements inside it, the information will be discarded
     */
    void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue );

    /**
     * @brief   The function returns a one if the queue is full, 0 otherwise
     */
    uint8_t HIL_QUEUE_IsFull( QUEUE_HandleTypeDef *hqueue );

    /**
     * @brief   Copies the information referenced by the empty pointer data to the buffer controlled by hqueue and operates with interrupt
     */
    uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr );

    /**
     * @brief   Reads data from the buffer controlled by hqueue, the data is copied into the data type referenced by the empty pointer data  and operates with interrupt
     */
    uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr );

    /**
     * @brief   The function returns a one if there are no more elements that can be read from the queue and zero if there is at least one element that can be read.
     */
    uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr );

    /**
     * @brief   The function must empty the queue in case it has elements inside it, the information will be discarded and operates wih interrupt
     */
    void HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr );

    /**
     * @brief Struct variable of Queue elements to develop in clock
    */
    extern QUEUE_HandleTypeDef ClockQueue;

    /**
     * @brief Struct variable of Queue elements to develop in DISPLAY
    */
    extern QUEUE_HandleTypeDef DisplayQueue;
    
#endif