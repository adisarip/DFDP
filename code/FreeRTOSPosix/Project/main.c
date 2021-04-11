
// Simulating and testing the results for Data Freshness with Deferred Pre-emption

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include "dfdptask.h"

int main (int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("[ERROR] Input data missing.\n");
        printf("[USAGE] ./simrtos <no_of_tasks> <priority_order_with_fnr_lengths> <input_taskset_file>\n");
        printf("        Eg: ./simrtos 4 1:1,3:1,4:1,2:21 taskset.txt\n");
        fflush(stdout);
        return 0;
    }

    int task_count = atoi(argv[1]);
    if (task_count > MAX_RTOS_SCHEDULER_TASKS)
    {
        printf("[ERROR] Max number of RTOS tasks supported are %d\n", MAX_RTOS_SCHEDULER_TASKS);
        printf("[INFO]  Re-configure the setting 'MAX_RTOS_SCHEDULER_TASKS' to support additional tasks\n");
        fflush(stdout);
        return 0;
    }

    char* priority_order = argv[2];
    char* input_file = argv[3];
    TaskData task_data[task_count];

    create_task_dataset(task_count,
                        priority_order,
                        input_file,
                        task_data);

    for (int i=0; i<task_count; i++)
    {
        char sTaskLabel[20];
        sprintf(sTaskLabel, "Task-%d", i+1);
        xTaskCreate(run_task,
                    sTaskLabel,
                    1024,
                    (void*) &task_data[i],
                    task_data[i].priority,
                    &xTask[i]);
    }

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
