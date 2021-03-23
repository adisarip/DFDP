
// Simulating and testing the results for Data Freshness with Deferred Pre-emption

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
void fill_priority_data(int task_count, char* p_priority_order, TaskData* p_task_data);
void fill_task_data(int task_index, char* p_line, TaskData* p_task_data);
void create_task_dataset(int task_count, char* p_priority_order, char* p_filename, TaskData* p_task_data);
void run_task(void* data);

// function definitions
void fill_priority_data(int task_count,
                        char* p_priority_order,
                        TaskData* p_task_data)
{
    char delim[] = ",:";
    char* ptoken = strtok(p_priority_order, delim);
    int task_priority = 1;
    for (int idx=0; ((idx < task_count) && (ptoken != NULL)); idx++)
    {
        p_task_data[idx].id = atoi(ptoken);
        ptoken = strtok(NULL, delim);
        p_task_data[idx].fnr_length = atoi(ptoken);
        p_task_data[idx].priority = task_priority++;
        ptoken = strtok(NULL, delim);
    }
}

void fill_task_data(int task_index, char* p_line, TaskData* p_task_data)
{
    char* p_token = strtok(p_line, ",");
    if (p_token != NULL)
    {
        p_task_data[task_index].id = atoi(p_token);
        p_token = strtok(NULL, ",");
        p_task_data[task_index].period = pdMS_TO_TICKS(atoi(p_token));
        p_token = strtok(NULL, ",");
        p_task_data[task_index].execution_time = pdMS_TO_TICKS(atoi(p_token));
        p_token = strtok(NULL, ",");
        p_task_data[task_index].deadline = pdMS_TO_TICKS(atoi(p_token));
    }
}

void create_task_dataset(int task_count,
                         char* p_priority_order,
                         char* p_filename,
                         TaskData* p_task_data)
{
    FILE* fp = fopen(p_filename, "r");
    if (fp == NULL)
    {
        printf("[RERR] Input filename '%s' do not exist\n", p_filename);
        return;
    }
    char* line = NULL;
    size_t line_len = 0;
    ssize_t read_len = 0;
    int task_index = 0;

    while ((read_len = getline(&line, &line_len, fp)) != -1)
    {
        if (line[0] == '#')
        {
            // skip the line starting with '#' they are comments
            continue;
        }
        // remove the trailing new-line character
        line[strcspn(line, "\n")] = 0;
        fill_task_data(task_index, line, p_task_data);
        task_index++;
    }
    fclose(fp);
    if (line)
    {
        free(line);
    }

    // Fill the task dataset with priority order and fnr lengths
    fill_priority_data(task_count, p_priority_order, p_task_data);

    // display the task data
    for (int i=0; i<task_count; i++)
    {
        printf("[RTOS] Task-%u: period(%u) execution_time(%u) deadline(%u) priority(%u) fnr_length(%u)\n",
                p_task_data[i].id, p_task_data[i].period, p_task_data[i].execution_time, p_task_data[i].deadline,
                p_task_data[i].priority, p_task_data[i].fnr_length);
    }
    fflush(stdout);

}

void run_task(void* data)
{
    printf("[RTOS] Start DFDP evaluation ...%p\n", data);
    fflush(stdout);
    vTaskDelete(NULL);
}

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
    char* priority_order = argv[2];
    char* input_file = argv[3];
    TaskData task_data[task_count];

    create_task_dataset(task_count, priority_order, input_file, task_data);

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
