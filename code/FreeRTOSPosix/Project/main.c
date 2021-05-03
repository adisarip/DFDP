
// Simulating and testing the results for Data Freshness with Deferred Pre-emption

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dfdptask.h"

// helper functions declarations
void fill_priority_data(int task_count,
                        char* p_priority_order,
                        TaskData* p_task_data);
void fill_task_data(int task_index,
                    char* p_line,
                    TaskData* p_task_data);
FreshData generate_freshness_data(void);
void print_task_data(TaskData* p_data);


//============================ Main interface function definitons ============================//
void create_task_dataset(int task_count,
                         char* p_priority_order,
                         char* p_filename,
                         TaskData* p_task_data)
{
    FILE* fp = fopen(p_filename, "r");
    if (fp == NULL)
    {
        printf("[RERR] Input filename '%s' do not exist\n", p_filename);
        fflush(stdout);
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
}


void run_task(void* data)
{
    TaskData* p_task_data = (TaskData*)data;
    print_task_data(p_task_data);
    TickType_t current_tick_count = xTaskGetTickCount();

    //BaseType_t xResult;
    if (p_task_data->index == 0)
    {
        // first task to be started by the scheduler
        FreshData fresh_data = generate_freshness_data();
        xTaskNotify(xTask[p_task_data->index+1],
                    fresh_data.fdata,
                    eSetValueWithOverwrite);
        while(1)
        {
            TickType_t tick_count = xTaskGetTickCount();
            if (tick_count >= current_tick_count + p_task_data->execution_time)
            {
                printf("Task-%u : Response Time : %d\n", p_task_data->id, tick_count);
                if (tick_count > p_task_data->period)
                {
                    printf("Task %u : DEADLINE VIOLATION !!!\n", p_task_data->id);
                }
                break;
            }
        }
    }
    else if (p_task_data->index > 0 &&
             p_task_data->index < p_task_data->task_count-1)
    {
        // remaining tasks except the last one
        uint32_t data_received = 0;
        xTaskNotifyWait(0, // No need to clear any bits in the notification while entering
                        0, // No need to clear any bits in the notification while exiting
                        &data_received, // variable to receive the notification data
                        portMAX_DELAY); // Wait for the notification indefinitely

        // perform some task specific computations
        // send the data to the next task
        bool isDataSent = false;
        while(1)
        {
            TickType_t tick_count = xTaskGetTickCount();
            if (!isDataSent && data_received != 0)
            {
                xTaskNotify(xTask[p_task_data->index+1],
                            data_received,
                            eSetValueWithOverwrite);
                isDataSent = true;
            }
            if (tick_count >= current_tick_count + p_task_data->execution_time)
            {
                printf("Task-%u : Response Time : %d\n", p_task_data->id, tick_count);
                if (tick_count > p_task_data->period)
                {
                    printf("Task %u : DEADLINE VIOLATION !!!\n", p_task_data->id);
                }
                break;
            }
        }
    }
    else
    {
        // last task
        uint32_t data_received;
        xTaskNotifyWait(0, // No need to clear any bits in the notification while entering
                        0, // No need to clear any bits in the notification while exiting
                        &data_received, // variable to receive the notification data
                        portMAX_DELAY); // Wait for the notification indefinitely
        while(1)
        {
            TickType_t tick_count = xTaskGetTickCount();
            if (data_received !=0)
            {
                // Get current timestamp
                TickType_t new_tick_count = xTaskGetTickCount();
                FreshData final_fresh_data;
                final_fresh_data.fdata = data_received;

                printf("[INFO] Data Freshness Summary:\n");
                printf("==============================\n");
                printf(" Received Data            = %X\n", final_fresh_data.fields.data);
                printf(" Timestamp while creation = %u\n", final_fresh_data.fields.timestamp);
                printf(" Timestamp when received  = %u\n", new_tick_count);
                printf(" Data Freshness quotient  = %u\n", new_tick_count - final_fresh_data.fields.timestamp);
                printf("==============================\n");
                break;
            }
            if (tick_count >= current_tick_count + p_task_data->execution_time)
            {
                printf("Task-%u : Response Time : %d\n", p_task_data->id, tick_count);
                if (tick_count > p_task_data->period)
                {
                    printf("Task %u : DEADLINE VIOLATION !!!\n", p_task_data->id);
                }
                break;
            }
        }
    }

    vTaskDelete(NULL);
}

//============================ Helper function definitons ============================//
void fill_task_data(int task_index,
                    char* p_line,
                    TaskData* p_task_data)
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

void fill_priority_data(int task_count,
                        char* p_priority_order,
                        TaskData* p_task_data)
{
    char delim[] = ",:";
    char* ptoken = strtok(p_priority_order, delim);
    int task_priority = task_count;
    for (int idx=0; ((idx < task_count) && (ptoken != NULL)); idx++)
    {
        p_task_data[idx].index = idx;
        p_task_data[idx].task_count = task_count;
        p_task_data[idx].id = atoi(ptoken);
        ptoken = strtok(NULL, delim);
        p_task_data[idx].fnr_length = atoi(ptoken);
        p_task_data[idx].priority = task_priority--;
        ptoken = strtok(NULL, delim);
    }
}

FreshData generate_freshness_data()
{
    // create data with a timestamp embedded in it and return
    FreshData fresh_data;
    fresh_data.fields.timestamp = xTaskGetTickCount();
    fresh_data.fields.data = 0xABCD;
    return fresh_data;
}

void print_task_data(TaskData* p_data)
{
    printf("[RTOS] Task-%u: [%u] period(%u) execution_time(%u) deadline(%u) priority(%u) fnr_length(%u)\n",
            p_data->id, p_data->index, p_data->period, p_data->execution_time,
            p_data->deadline, p_data->priority, p_data->fnr_length);
    fflush(stdout);
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
