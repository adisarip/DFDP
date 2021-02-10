
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"        /* RTOS firmware */
#include "task.h"            /* Task */
#include "timers.h"
#include "semphr.h"

#define uint unsigned int

// Task Handles
// Release Time             : t = 2
// Resource Acquire Request : t = 1
// Critical Section         : [R;2]
// Execution Time           : 4 units
// Priority                 : 3 (highest)
static TaskHandle_t xTask1 = NULL;

// Release Time             : t = 5
// Resource Acquire Request : ----
// Critical Section         : ----
// Execution Time           : 5 units
// Priority                 : 2 (highest)
static TaskHandle_t xTask2 = NULL;

// Release Time             : t = 0
// Resource Acquire Request : t = 1
// Critical Section         : [R;5]
// Execution Time           : 7 units
// Priority                 : 1 (lowest)
static TaskHandle_t xTask3 = NULL;

// Shared resource which tasks would like to acquire lock on
int xSharedResource = 0;
// A semaphore to lock the access on the above shared resource
SemaphoreHandle_t xSemaphore = NULL;

struct TaskData
{
    // Data
    uint       id;
    uint       priority;
    TickType_t release_time;
    TickType_t execution_time;
    TickType_t resource_start_time;
    TickType_t resource_duration;
};

enum
{
    TASK1 = 1001,
    TASK2 = 1002,
    TASK3 = 1003,
};

typedef struct TaskData TaskData;

// function declarations
void init(TaskData*, uint, uint, TickType_t, TickType_t, TickType_t, TickType_t);
void run_task1(void* data);
void run_task2(void* data);
void run_task3(void* data);

// function definitions

void run_task1(void* data)
{
    // get the task data
    TaskData* pTaskData = (TaskData*)data;
    // Adjust the release time
    vTaskDelay(pTaskData->release_time);
    TickType_t current_tick_count = xTaskGetTickCount();

    // Start task execution
    UBaseType_t xOrigPriority = uxTaskPriorityGet(NULL);
    UBaseType_t xPriority1;
    UBaseType_t xPriority2;
    UBaseType_t xPriority3;
    TickType_t tick_count = 0;
    TickType_t prev_tick_count = 0;
    uint isFirstPass = 1;
    TickType_t delta_ticks = 0;
    BaseType_t xSemaphoreFlag = pdFAIL;
    while (1)
    {
        tick_count = xTaskGetTickCount();
        if ((prev_tick_count != tick_count || isFirstPass) && tick_count%100 == 0)
        {
            xPriority1 = uxTaskPriorityGet(xTask1);
            xPriority2 = uxTaskPriorityGet(xTask2);
            xPriority3 = uxTaskPriorityGet(xTask3);
            if (isFirstPass)
            {
                printf("[Task %u] : [t=%02d] : [P1=%lu P2=%lu P3=%lu] Release Time (%d)\n",
                       pTaskData->id, tick_count/100, xPriority1, xPriority2, xPriority3, pTaskData->release_time/100);
            }
            else
            {
                printf("[Task %u] : [t=%02d] : [P1=%lu P2=%lu P3=%lu]\n",
                       pTaskData->id, tick_count/100, xPriority1, xPriority2, xPriority3);
            }
            prev_tick_count = tick_count;
            isFirstPass = 0;
        }
        if (tick_count >= current_tick_count + pTaskData->execution_time)
        {
            printf("[Task %u] : [t=%02d] : Execution ended.\n", pTaskData->id, tick_count/100);
            break;
        }
        else if (tick_count >= current_tick_count + pTaskData->resource_start_time + pTaskData->resource_duration &&
                 pdPASS == xSemaphoreFlag)
        {
            UBaseType_t xCurrPriority = uxTaskPriorityGet(NULL);
            if (xCurrPriority > xOrigPriority)
            {
                current_tick_count = current_tick_count + 100;
            }
            // release the semaphore
            printf("[Task %u] : [t=%02d] : Resource %d released\n", pTaskData->id, tick_count/100, xSharedResource);
            xSemaphoreGive(xSemaphore);
            xSemaphoreFlag = pdFAIL;

        }
        else if (tick_count == current_tick_count + pTaskData->resource_start_time &&
                 pdFAIL == xSemaphoreFlag)
        {
            // acquire lock on the shared resource
            // If lock not acquired then wait until you get the lock on the resource
            printf("[Task %u] : [t=%02d] : Resource Requested\n", pTaskData->id, tick_count/100);
            delta_ticks = xTaskGetTickCount();
            xSemaphoreFlag = xSemaphoreTake(xSemaphore, portMAX_DELAY);
            if (pdPASS == xSemaphoreFlag)
            {
                // Lock acquired by on the semaphore
                // work with the shared resource
                tick_count = xTaskGetTickCount();
                delta_ticks = tick_count - delta_ticks;
                current_tick_count = current_tick_count + delta_ticks;
                xSharedResource = TASK1;
                printf("[Task %u] : [t=%02d] : Resource acquired : Set Resource Value in Task %u : %d\n",
                       pTaskData->id, tick_count/100, pTaskData->id, xSharedResource);
            }
        }
    }
    vTaskDelete(NULL);
}

void run_task2(void* data)
{
    // get the task data
    TaskData* pTaskData = (TaskData*)data;
    // Adjust the release time
    vTaskDelay(pTaskData->release_time);
    TickType_t current_tick_count = xTaskGetTickCount();

    // Start task execution
    UBaseType_t xPriority1;
    UBaseType_t xPriority2;
    UBaseType_t xPriority3;
    TickType_t tick_count = 0;
    TickType_t prev_tick_count = 0;
    uint isFirstPass = 1;
    while (1)
    {
        tick_count = xTaskGetTickCount();
        if ((prev_tick_count != tick_count || isFirstPass) && tick_count%100 == 0)
        {
            xPriority1 = uxTaskPriorityGet(xTask1);
            xPriority2 = uxTaskPriorityGet(xTask2);
            xPriority3 = uxTaskPriorityGet(xTask3);
            if (isFirstPass)
            {
                printf("[Task %u] : [t=%02d] : [P1=%lu P2=%lu P3=%lu] Release Time (%d)\n",
                       pTaskData->id, tick_count/100, xPriority1, xPriority2, xPriority3, pTaskData->release_time/100);
            }
            else
            {
                printf("[Task %u] : [t=%02d] : [P1=%lu P2=%lu P3=%lu]\n",
                       pTaskData->id, tick_count/100, xPriority1, xPriority2, xPriority3);
            }
            prev_tick_count = tick_count;
            isFirstPass = 0;
        }
        if (tick_count >= current_tick_count + pTaskData->execution_time)
        {
            printf("[Task %u] : [t=%02d] : Execution ended.\n", pTaskData->id, tick_count/100);
            break;
        }
    }
    vTaskDelete(NULL);
}

void run_task3(void* data)
{
    // get the task data
    TaskData* pTaskData = (TaskData*)data;
    // No release time adjustment needed - as release time is "t = 0"
    TickType_t current_tick_count = xTaskGetTickCount();

    // Start task execution
    UBaseType_t xOrigPriority = uxTaskPriorityGet(NULL);
    UBaseType_t xPriority1;
    UBaseType_t xPriority2;
    UBaseType_t xPriority3;
    TickType_t tick_count = 0;
    TickType_t prev_tick_count = 0;
    uint isFirstPass = 1;
    TickType_t delta_ticks = 0;
    BaseType_t xSemaphoreFlag = pdFAIL;
    while (1)
    {
        tick_count = xTaskGetTickCount();
        if ((prev_tick_count != tick_count || isFirstPass) && tick_count%100 == 0)
        {
            xPriority1 = uxTaskPriorityGet(xTask1);
            xPriority2 = uxTaskPriorityGet(xTask2);
            xPriority3 = uxTaskPriorityGet(xTask3);
            if (isFirstPass)
            {
                printf("[Task %u] : [t=%02d] : [P1=%lu P2=%lu P3=%lu] Release Time (%d)\n",
                       pTaskData->id, tick_count/100, xPriority1, xPriority2, xPriority3, pTaskData->release_time/100);
            }
            else
            {
                printf("[Task %u] : [t=%02d] : [P1=%lu P2=%lu P3=%lu]\n",
                       pTaskData->id, tick_count/100, xPriority1, xPriority2, xPriority3);
            }
            if (tick_count/100 - prev_tick_count/100 > 1 && isFirstPass == 0)
            {
                current_tick_count += tick_count - prev_tick_count - 100;
            }
            prev_tick_count = tick_count;
            isFirstPass = 0;
        }
        if (tick_count >= current_tick_count + pTaskData->execution_time)
        {
            printf("[Task %u] : [t=%02d] : Execution ended.\n", pTaskData->id, tick_count/100);
            break;
        }
        else if (tick_count >= current_tick_count + pTaskData->resource_start_time + pTaskData->resource_duration &&
                 pdPASS == xSemaphoreFlag)
        {
            UBaseType_t xCurrPriority = uxTaskPriorityGet(NULL);
            if (xCurrPriority > xOrigPriority)
            {
                current_tick_count = current_tick_count + 100;
            }
            // release the semaphore
            printf("[Task %u] : [t=%02d] : Resource %d released\n", pTaskData->id, tick_count/100, xSharedResource);
            xSemaphoreGive(xSemaphore);
            xSemaphoreFlag = pdFAIL;
        }
        else if (tick_count == current_tick_count + pTaskData->resource_start_time &&
                 pdFAIL == xSemaphoreFlag)
        {
            // acquire lock on the shared resource
            // If lock not acquired then wait until you get the lock on the resource
            printf("[Task %u] : [t=%02d] : Resource Requested\n", pTaskData->id, tick_count/100);
            delta_ticks = xTaskGetTickCount();
            xSemaphoreFlag = xSemaphoreTake(xSemaphore, portMAX_DELAY);
            if (pdPASS == xSemaphoreFlag)
            {
                // Lock acquired by on the semaphore
                // work with the shared resource
                tick_count = xTaskGetTickCount();
                delta_ticks = tick_count - delta_ticks;
                current_tick_count = current_tick_count + delta_ticks;
                xSharedResource = TASK3;
                printf("[Task %u] : [t=%02d] : Resource acquired : Set Resource Value in Task %u : %d\n",
                       pTaskData->id, tick_count/100, pTaskData->id, xSharedResource);
            }
        }
    }
    vTaskDelete(NULL);
}

void init(TaskData* pTaskData,
          uint _id,
          uint _priority,
          TickType_t _release_time,
          TickType_t _execution_time,
          TickType_t _resource_start_time,
          TickType_t _resource_duration)
{
    pTaskData->id = _id;
    pTaskData->priority = _priority;
    pTaskData->release_time = _release_time;
    pTaskData->execution_time = _execution_time;
    pTaskData->resource_start_time = _resource_start_time;
    pTaskData->resource_duration = _resource_duration;
}

int main (void)
{
    printf("\n");
    printf("Priority Inheritance Protocol Using \"Semaphore Mutex\" in FreeRTOS:\n");
    printf("------------------------------------------------------------------\n");
    fflush(stdout);

    TaskData sTaskData1, sTaskData2, sTaskData3;
    //    function,  id, priority,   release time,       execution time,     resource start,   resource duration
    init(&sTaskData1, 1,    3,     pdMS_TO_TICKS(200), pdMS_TO_TICKS(400), pdMS_TO_TICKS(100), pdMS_TO_TICKS(200));
    init(&sTaskData2, 2,    2,     pdMS_TO_TICKS(500), pdMS_TO_TICKS(500), pdMS_TO_TICKS(000), pdMS_TO_TICKS(000));
    init(&sTaskData3, 3,    1,     pdMS_TO_TICKS(000), pdMS_TO_TICKS(700), pdMS_TO_TICKS(100), pdMS_TO_TICKS(500));
    xTaskCreate(run_task1, "Task 1", 1024, (void*) &sTaskData1, sTaskData1.priority, &xTask1);
    xTaskCreate(run_task2, "Task 2", 1024, (void*) &sTaskData2, sTaskData2.priority, &xTask2);
    xTaskCreate(run_task3, "Task 3", 1024, (void*) &sTaskData3, sTaskData3.priority, &xTask3);
    xSemaphore = xSemaphoreCreateMutex();
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
