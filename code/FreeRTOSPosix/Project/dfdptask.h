
// Imlementing Data Freshness with Deferred Premption
// by computing the final non-premptive region for each task

#ifndef DFDPTASK_H
#define DFDPTASK_H

/* Standard includes. */
#include <stdbool.h>
#include "FreeRTOS.h" /* RTOS firmware */
#include "task.h"     /* Task */

#define uint unsigned int
#define MAX_RTOS_SCHEDULER_TASKS 20

struct TaskData
{
    // Data
    uint       id;
    uint       index;
    uint       task_count;
    uint       priority;
    uint       fnr_length;
    TickType_t release_time;
    TickType_t period;
    TickType_t execution_time;
    TickType_t deadline;
};
typedef struct TaskData TaskData;

union FreshData
{
    uint32_t fdata;
    struct
    {
        // TickType_t is a typedef for uint16_t if configUSE_16_BIT_TICKS is set to '1'
        // So in the source all the TickType_t will be a 16-bit tick values.
        uint16_t timestamp;
        uint16_t data;
    }fields;
};
typedef union FreshData FreshData;

static TaskHandle_t xTask[MAX_RTOS_SCHEDULER_TASKS];

// function declarations
void create_task_dataset(int task_count,
                         char* p_priority_order,
                         char* p_filename,
                         TaskData* p_task_data);
void run_task(void* data);

#endif // DFDPTASK_H
