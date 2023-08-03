/**
 * @file  scheduler.c
 * @brief This file contains the functions that govern the operation of the scheduler.
*/

#include "app_bsp.h"

/**
 * @brief Structure for the TIM6 handler.
*/
extern TIM_HandleTypeDef TIM6_Handler;
TIM_HandleTypeDef TIM6_Handler = {0};

/**
 * @brief Function to initialice the task count.
 * The hscheduler structure initialices with the values passed as parameters.
 * @param[in] hscheduler
*/
void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler )
{
    assert_error( ( hscheduler->tasks != 0u ), SCHEDULER_PAR_ERROR ); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.'*/
    assert_error( ( hscheduler->tick != 0u ), SCHEDULER_PAR_ERROR );  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    
    hscheduler->tasksCount = 0;
}

/**
 * @brief Function to register the number of tasks. 
 * The tasks are assigned a number starting from #1 to N.
 * @param[in] hscheduler
 * @param[in] InitPtr
 * @param[in] TaskPtr
 * @param[in] Period
 * @retval 	The function returns the Task ID of the input task.
*/
uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period )
{
    uint8_t TaskID = 0;

    assert_error( ( hscheduler->tasks != 0u ), SCHEDULER_PAR_ERROR ); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->tick != 0u ), SCHEDULER_PAR_ERROR );  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

    if( ( Period >= hscheduler-> tick ) && ( Period % hscheduler->tick ) == ( uint32_t ) 0 )
    {   
        hscheduler->taskPtr[hscheduler->tasksCount].period = Period;
        hscheduler->taskPtr[hscheduler->tasksCount].elapsed= 0;
        hscheduler->taskPtr[hscheduler->tasksCount].initFunc = InitPtr;
        hscheduler->taskPtr[hscheduler->tasksCount].taskFunc = TaskPtr;

        hscheduler->tasksCount++;

        TaskID = hscheduler->tasksCount;
    }

    return TaskID;
}

/**
 * @brief Function that stops any of the previously registered tasks. 
 * The function saves in an auxiliar variable the task that was stopped before "erasing" it. 
 * @param[in] hscheduler
 * @param[in] task
 * @retval 	The function returns a flag: true if the task was stopped successfully, and false if it wasn't.
*/
uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task )
{
    uint8_t SuccessFlag = 0;

    assert_error( ( hscheduler->tasks != 0u ), SCHEDULER_PAR_ERROR );                 /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->tick != 0u ), SCHEDULER_PAR_ERROR );                  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->period != 0u ), SCHEDULER_PAR_ERROR );       /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->initFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->taskFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

    if ( task <= hscheduler->tasksCount )
    {   /*Saving the function before stopping it.*/
        hscheduler->taskPtr[task - ( uint32_t ) 1].taskFunc1 = hscheduler->taskPtr[task - ( uint32_t ) 1 ].taskFunc;
        /*Task is stopped.*/
        hscheduler->taskPtr[task - ( uint32_t ) 1].taskFunc = NULL;
        SuccessFlag = 1;
    }

    return SuccessFlag;
}

/**
 * @brief Function that starts a previously stopped task.
 * 
 * @param[in] hscheduler
 * @param[in] task
 * @retval 	The function returns a flag: true if the task was started successfully, and false if it wasn't.
*/
uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task )
{
    uint8_t SuccessFlag = 0;

    assert_error( ( hscheduler->tasks != 0u ), SCHEDULER_PAR_ERROR );                 /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->tick != 0u ), SCHEDULER_PAR_ERROR );                  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->period != 0u ), SCHEDULER_PAR_ERROR );       /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->initFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->taskFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

    if (task <= hscheduler->tasksCount)
    {   /*The function that was previuouly saved before it was stopped is assigned again.*/
        hscheduler->taskPtr[task - ( uint32_t ) 1 ].taskFunc = hscheduler->taskPtr[task - ( uint32_t ) 1 ].taskFunc1;

        SuccessFlag = 1;
    }

    return SuccessFlag; 
}

/**
 * @brief Function that changes the period of execution of a given previously registered task.
 * 
 * @param[in] hscheduler
 * @param[in] task
 * @param[in] period
 * @retval 	The function returns a flag: true if the task was changed successfully, and false if it wasn't.
*/
uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period )
{
    uint8_t SuccessFlag = 0;

    assert_error( ( hscheduler->tasks != 0u ), SCHEDULER_PAR_ERROR );                 /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->tick != 0u ), SCHEDULER_PAR_ERROR );                  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->period != 0u ), SCHEDULER_PAR_ERROR );       /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->initFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->taskFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

    if ( ( period % hscheduler->tick ) == ( uint32_t ) 0 )
    {   
        hscheduler->taskPtr[task - ( uint32_t ) 1].period = period;
        SuccessFlag = 1;
    }

    return SuccessFlag;
}

/**
 * @brief Function that initialices & runs the registered tasks.
 * 
 * @param[in] hscheduler
*/
void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler )
{
    assert_error( ( hscheduler->tasks != 0u ), SCHEDULER_PAR_ERROR );                 /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->tick != 0u ), SCHEDULER_PAR_ERROR );                  /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->period != 0u ), SCHEDULER_PAR_ERROR );       /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->initFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/
    assert_error( ( hscheduler->taskPtr->taskFunc != NULL ), SCHEDULER_PAR_ERROR );   /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

    __HAL_RCC_TIM6_CLK_ENABLE();

    /*Clock APB1 = 32 MHz, Prescaler = 32000 -> APB1/Prescaler = 1 kHz ∴ t = 1 ms*/
    TIM6_Handler.Instance = TIM6;
    TIM6_Handler.Init.Prescaler = 32000;                   /*APB1/Prescaler = 1 kHz ∴ t = 1 ms*/
    TIM6_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    /*Count from zero to TIM6_Handler.Init.Period*/
    TIM6_Handler.Init.Period = 0xFFFF;                     /*Count Overflow at 65535 (maximum).*/
    TIM6_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    TIM6_Handler.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init( &TIM6_Handler );

    /*Timer starts counting.*/
    HAL_TIM_Base_Start( &TIM6_Handler );

    uint32_t TimerReading;
    uint32_t PreviousTime[TASKS_N] = {0};
    uint32_t Difference;
    uint32_t Deviation;
    uint32_t UpperLimit;
    uint32_t LowerLimit;

    uint32_t tickstart = HAL_GetTick();

    /*Execution of the task initialization routines.*/
    for ( uint32_t i = 0; i < hscheduler->tasksCount; i++ )
    {           
        if( hscheduler->taskPtr[i].initFunc != NULL ) {
            hscheduler->taskPtr[i].initFunc();
        }
    }

    /*Execution of the tasks.*/
    while (1)
    {
        if ( ( HAL_GetTick() - tickstart ) >= hscheduler->tick )
        {
            tickstart = HAL_GetTick();

            for ( uint32_t i = 0; i < hscheduler->tasksCount; i++ )
            {
                if( hscheduler->taskPtr[i].taskFunc != NULL )
                {
                    hscheduler->taskPtr[i].elapsed += hscheduler->tick;
                    if( hscheduler->taskPtr[i].elapsed >= hscheduler->taskPtr[i].period ) 
                    {
                        TimerReading  = __HAL_TIM_GET_COUNTER( &TIM6_Handler );
                        Difference = TimerReading - PreviousTime[i];
                        Deviation = (hscheduler->taskPtr[i].period * 10u) / 100u;

                        UpperLimit = hscheduler->taskPtr[i].period + Deviation;
                        LowerLimit = hscheduler->taskPtr[i].period - Deviation;

                        assert_error( ! ( ( Difference >= LowerLimit ) && ( Difference <= UpperLimit ) ), SCHEDULER_TASK_ERROR ); /*cppcheck-suppress misra-c2012-11.8 ; Function can't be modified.*/

                        hscheduler->taskPtr[i].elapsed = 0;
                        hscheduler->taskPtr[i].taskFunc();

                        PreviousTime[i] = TimerReading;
                    }
                }
            }

        }
    }
}