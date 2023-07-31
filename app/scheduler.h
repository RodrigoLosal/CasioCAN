/**
 * @file scheduler.h
 * @brief This file initialices the functions & variables of the scheduler.
*/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "app_bsp.h"

/**
 * @brief Structure with the TCB elements.
*/
typedef struct _task
{
    uint32_t period;          /*!<How often the task shopud run in ms.*/
    uint32_t elapsed;         /*!<The cuurent elapsed time.*/
    void (*initFunc)(void);   /*!<Pointer to init task function.*/
    void (*taskFunc)(void);   /*!<Pointer to task function.*/
    void (*taskFunc1)(void); /*!<Pointer to task function auxiliary.*/
} Task_TypeDef;

/**
 * @brief Struct of scheduler control
*/
typedef struct _scheduler
{
    uint32_t tasks;         /*!<number of task to handle*/
    uint32_t tick;          /*!<the time base in ms*/
    uint32_t tasksCount;    /*!<internal task counter*/
    Task_TypeDef *taskPtr;  /*!<Pointer to buffer for the TCB tasks*/

}Scheduler_HandleTypeDef;

/**
 * @brief Function to INITIALIZE the task 
 * Function to initialize all the things required the scheduler to start working
*/
void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler );

/**
 * @brief Function to register the tasks 
 * Function to add the task to bufer
*/
uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period );

/**
 * @brief Function to stop the tasks 
 * Function to stop some task
*/
uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );

/**
 * @brief Function to start the stop previous task
 * Function to start again some task
*/
uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );

/**
 * @brief Function to change the period
 * Function to add new period
*/
uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period );

/**
 * @brief Function to run all task
 * Run the different tasks that have been registered
*/
void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler );

#endif