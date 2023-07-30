/**
 * @file    hil_queue.h
 * @brief   **This file initialices the functions and variables for the circular buffer.**
 * 
 */

#ifndef HIL_QUEUE_H
#define HIL_QUEUE_H

    #include "app_bsp.h"

    /** 
     * @defgroup <Definition of the possible queue results & number of elements.>
     * 
     @{ */
    #define     QUEUE_OK            0   /*!< Queue worked correctly.*/
    #define     QUEUE_NOT_OK        1   /*!< Queue didn't work correctly.*/
    #define     QUEUE_ELEMENTS      9   /*!< Number of possible messages to save in determined timing.*/
    /**
     @} */

    /**
     * @brief Structure with the elements of the circular buffer.
    */
    typedef struct
    {
        void        *Buffer;        /**< Pointer to the memory space used as buffer by the queue.        */
        uint32_t    Elements;       /**< Number of elements to store (queue size).                */
        uint8_t     Size;           /**< Size of the type of elements to store.                           */
        uint32_t    Head;           /**< Pointer indicating the next space to write.                 */
        uint32_t    Tail;           /**< Pointer indicating the next space to read.                     */
        uint8_t     Empty;          /**< Flag that indicates if there are no elements to read.                      */
        uint8_t     Full;           /**< Flag that indicates if it is not possible to continue writing more elements. */
        uint32_t    SavedElements;   /**< Saved elements. */
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