
// Simulating and testing the results for Data Freshness with Deferred Pre-emption

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h" /* RTOS firmware */
#include "task.h"     /* Task */

#define uint unsigned int

struct TaskData
{
    // Data
    uint       id;
    uint       priority;
    uint       fnr_length;
    TickType_t release_time;
    TickType_t period;
    TickType_t execution_time;
    TickType_t deadline;
};

// Task Handles
static TaskHandle_t xTask1 = NULL;

typedef struct TaskData TaskData;

// function declarations
void init(TaskData*, uint, uint, TickType_t, TickType_t, TickType_t, TickType_t);
void get_priority_order_using_fpds(char* filename);
void run_task(void* data);

// function definitions

void get_priority_order_using_fpds(char* filename)
{
    // run FPDS from here
    printf("Get priority order using FPDS ... %p\n", filename);
    fflush(stdout);
}

void run_task(void* data)
{
    printf("Start DFDP evaluation ...%p\n", data);
    fflush(stdout);
    vTaskDelete(NULL);
}

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("[ERROR] Input data file missing.\n");
        printf("[USAGE] ./freesim <taskset_input_file>\n");
        fflush(stdout);
        return 0;
    }
    get_priority_order_using_fpds(argv[1]);
    xTaskCreate(run_task, "Task 1", 1024, NULL, 1, &xTask1);
    vTaskStartScheduler();
    return 0;
}

/*-----------------------------------------------------------*/

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
     taskENTER_CRITICAL();
    {
        printf("[ASSERT] %s:%lu\n", pcFileName, ulLine);
        fflush(stdout);
    }
    taskEXIT_CRITICAL();
    exit(-1);
}
