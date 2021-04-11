
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

    //BaseType_t xResult;
    if (p_task_data->index == 0)
    {
        // first task to be started by the scheduler
        FreshData fresh_data = generate_freshness_data();
        xTaskNotify(xTask[p_task_data->index+1],
                    fresh_data.fdata,
                    eSetValueWithOverwrite);
    }
    else if (p_task_data->index > 0 &&
             p_task_data->index < p_task_data->task_count-1)
    {
        // remaining tasks except the last one
        uint32_t data_received;
        xTaskNotifyWait(0, // No need to clear any bits in the notification while entering
                        0, // No need to clear any bits in the notification while exiting
                        &data_received, // variable to receive the notification data
                        portMAX_DELAY); // Wait for the notification indefinitely

        // perform some task specific computations
        // send the data to the next task

        xTaskNotify(xTask[p_task_data->index+1],
                    data_received,
                    eSetValueWithOverwrite);
    }
    else
    {
        // last task
        uint32_t data_received;
        xTaskNotifyWait(0, // No need to clear any bits in the notification while entering
                        0, // No need to clear any bits in the notification while exiting
                        &data_received, // variable to receive the notification data
                        portMAX_DELAY); // Wait for the notification indefinitely

        // Get current timestamp
        TickType_t current_tick_count = xTaskGetTickCount();
        FreshData final_fresh_data;
        final_fresh_data.fdata = data_received;

        printf("[INFO] Data Freshness Summary:\n");
        printf("==============================\n");
        printf(" Received Data            = %X\n", final_fresh_data.fields.data);
        printf(" Timestamp while creation = %u\n", final_fresh_data.fields.timestamp);
        printf(" Timestamp when received  = %u\n", current_tick_count);
        printf(" Data Freshness quotient  = %u\n", current_tick_count - final_fresh_data.fields.timestamp);
        printf("==============================\n");
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
