
# Data Freshness with Deferred Pre-emption

## Introduction
**_Data Freshness:_**
> In many application scenarios, data consumed by real-time tasks are required to meet a maximum age, or freshness, guarantee. Data freshness talks about the end-to-end freshness constraint of data that is passed along a chain of tasks in a uni-processor setting.

**_Fixed Priority with Deferred Pre-emption (FPDS):_**
> The schedulability of systems using fixed priority pre-emptive scheduling can be improved by the use of non-preemptive regions at the end of each task’s execution; an approach referred to as deferred pre-emption. Choosing the appropriate length for the final non pre-emptive region of each task is a trade-off between improving the worst-case response time of the task itself and increasing the amount of blocking imposed on higher priority tasks. With this technique we can compute both the optimal priority ordering of tasks and and the lengths of their final non pre-emptive regions.

**_Data Freshness with Deferred Pre-emption (DFDP):_**
> Data Freshness with Deferred Pre-emption  is an attempt to combine both the ideas discussed above to attain an better freshness quotient in the case of task chains with a uni-processor setting **(we can add some more lines here - need help in that).**

## Environment Details
> Operating System:  _Mac OSX / Linux_  
> Package Installer : _Homebrew/apt-get_  
> Compiler : _gcc/g++_  
> Scripting : _python3_  
> Additonal Packages : _FreeRTOS Posix Simulator_  

## Solution Approach

 1. Idea is to run FPDS algorithm for a given set of chained taskset and compute the optimal priority ordering for the taskset and the final non pre-emptive region (FNR) lengths for each task.  
 2. Pass the computed priority ordering and the FNR lengths of the taskset to FreeRTOS simulator.  
 3. Create tasks in FreeRTOS simulator and store the task related data in them (like period, execution time, deadline), along with priorities and FNR lengths.  
 4. FreeRTOS Scheduler will schedule the taskset (based on the priorities) provided.  
 5. Then using task notifications feature of FreeRTOS, we can pass some data and its creation time embedded within a 32-bit unsigned integer value and pass through all the tasks till the last task.  
 6. Once this data object (notification) reaches the last task, we will compute the current time and see the difference between created time (stored in the data object) and current time which is nothing but the freshness quotient.  
 7. From there on we will perform some tests and try to make use of the FNR lengths.  
 8. The FreeRTOS implementation doesn't support pre-emptive scheduling by default, hence I have implemented the support of it in the current source.
 9. Enabling pre-emptive scheduling in FreeRTOS by setting the option configUSE_PREEMPTION in FreeRTOSConfig.h file will enable the scheduler to schedule the tasks in pre-emptive manner, however while simulation the individual tasks would need to manually set the task states (based on the priorities) in such a manner that he scheduler is able to preemt the task which are currently in blocked/not-running state and schedule other tasks which are in ready state.  
 10. THREE (3) types of tasks we will encounter in any task set.  
     Type-1: Task with highest priority.  
     Type-2: Task with lowest priority.  
     Type-3: Tasks with priorities in between above two.  
 11. The Type-1 task with the highest priority will always be run on the scheduler with the immediate tick after it is released. Thus for this task, we only need to make sure that the - after the execution time of the task is over, it should sent to blocked state using the function *vTaskDelay(period - execution_time)*. This will move the task into blocked state and enable the scheduler to schedule other lower priority tasks. Note: Type-2 task should send a notification before it goes into blocked state to the next task priority-queue. The content of the notification will be the some data component and the timer-ticks of the current tick count where it is going into blocked state.  
 12. For the Type-3 tasks, one of the task in this section, will be scheduled by the scheduler and will receive the notification from the higher priority task with the timer data. This data will help compute the initial pre-empted timer ticks for that task (from its release time till it was acually scheduled on the scheduler). There might be cases where this task(s) need to be further pre-empted in future (if the higher priority task comes out from its blocked state). In that case there are checks in place which will compute the pre-empted timer ticks and add them to the execution time for the current task. Finally go into blocked state by executing *vTaskDelay(period - execution_time - preempted_ticks)*. Before, going into blocked state the task will forward the notification received to the next task in the priority-queue.  
 13. For the Type-2 task(with the lowest priority), everything is similar to type-3 task. Only difference is after the execution it will print the freshness data received from the higher protity task, received via notification.  
 14. The frame-work is in testing phase and might need some modifications to the code/framework based on test results. Stay tumed for updates.  

## Source Tree
>- Given below is the source tree for the current assignment.
>```
> code$ tree
> .
> ├── FPDS
> │   ├── Makefile
> │   ├── bin
> │   │   └── README
> │   ├── obj
> │   │   └── README
> │   └── src
> │       ├── FPDS.C
> │       ├── FPDS.H
> │       └── TestFPDS.C
> ├── FreeRTOSPosix
> │   ├── Makefile
> │   ├── Project
> │   │   ├── FreeRTOSConfig.h
> │   │   ├── dfdptask.h
> │   │   └── main.c
> │   ├── README.md
> │   └── Source
> │       ├── <FreeRTOS simulator source>
> ├── README.md
> ├── run.py
> └── taskset.txt

>- 'FPDS/' : Source code computing the optimal fixed priority with deferred pre-emption and FNR lengths.
>- 'FreeRTOS/Project' : Directory containing source code using the FreeRTOS simulator and schedule chained tasksets for computing data freshness with deferred pre-emption.
>- 'run.py' : A wrapper script to build all the binaries (both FPDS and FreeRTOS) and execute the whole procedure mentioned in the solution approach above.

## Input Data Format & Execution Runs

**_Sample input file format:_**
>```
> # Task set for running FNR-PA Algorithm for FPDS scheduling
> # task_id, task_period, task_execution_time, task_relative_deadline
> #
> # Task Set-1 (Given in Paper):
> 1,250,100,175
> 2,400,100,300
> 3,350,100,325
>```

**_Sample Execution Run:_**
>- Use the wrapper script **run.py** to build and execute the binaries.
> **./run.py <input_task_data_file\>**
>
>```
> code$ ./run.py taskset.txt
> [INFO] Building FPDS binaries ...
> [MAKE] Cleaning all the object files and binaries.
> [MAKE] Compiled src/FPDS.C successfully.
> [MAKE] Compiled src/TestFPDS.C successfully.
> [MAKE] Linking Complete.
> [INFO] Building RTOS simulator binaries ...
> [MAKE] CLEAN COMPLETE
> [MAKE] Compiling with gcc croutine.c
> [MAKE] Compiling with gcc event_groups.c
> [MAKE] Compiling with gcc list.c
> [MAKE] Compiling with gcc queue.c
> [MAKE] Compiling with gcc tasks.c
> [MAKE] Compiling with gcc timers.c
> [MAKE] Compiling with gcc heap_3.c
> [MAKE] Compiling with gcc port.c
> [MAKE] Compiling with gcc main.c
> [MAKE] Linking simrtos...
> [MAKE] BUILD COMPLETE: simrtos
> [INFO] Simulating Data-Freshness on FreeRTOS
> Running as PID: 47751
> Timer Resolution for Run TimeStats is 100 ticks per second.
> [RTOS] Task-1: [0] period(300) execution_time(100) deadline(200) priority(3) fnr_length(1)
> [Task-1] Released : 0
> [Task-1] Running with #ticks = 50 || 100
> [Task-1] Running with #ticks = 100 || 100
> [Task-1] Execution Completed. Response Time : 100
> Task[0] --> sending data to --> Task[1]
> [RTOS] Task-2: [1] period(500) execution_time(150) deadline(300) priority(2) fnr_length(1)
> [Task-2] Released : 0
> [Task-2] Pre-empted for 100 ticks [0 - 100]
> [Task-2] Running with #ticks = 100 || 250
> [Task-2] Running with #ticks = 150 || 250
> [Task-2] Running with #ticks = 200 || 250
> [Task-2] Running with #ticks = 250 || 250
> [Task-2] Execution Completed. Response Time : 250
> Task[1] --> sending data to --> Task[2]
> [RTOS] Task-3: [2] period(500) execution_time(100) deadline(500) priority(1) fnr_length(1)
> [Task-3] Released : 0
> [Task-3] Pre-empted for 250 ticks [0 - 250]
> [Task-3] Running with #ticks = 250 || 350
> [Task-1] Released : 300
> [Task-1] Running with #ticks = 300 || 400
> [Task-1] Running with #ticks = 350 || 400
> [Task-1] Running with #ticks = 400 || 400
> [Task-1] Execution Completed. Response Time : 100
> Task[0] --> sending data to --> Task[1]
> [Task-3] Pre-empted for 351 ticks [299 - 400]
> [Task-3] Running with #ticks = 400 || 451
> [Task-3] Running with #ticks = 450 || 451
> [Task-3] Execution Completed. Response Time : 451
> [Task-2] Released : 500
> [Task-2] Running with #ticks = 500 || 650
> [Task-2] Running with #ticks = 550 || 650
> [Task-1] Released : 600
> [Task-1] Running with #ticks = 600 || 700
> [Task-1] Running with #ticks = 650 || 700
> [Task-1] Running with #ticks = 700 || 700
> [Task-1] Execution Completed. Response Time : 100
> Task[0] --> sending data to --> Task[1]
> [Task-2] Pre-empted for 101 ticks [599 - 700]
> [Task-2] Running with #ticks = 700 || 751
> [Task-2] Running with #ticks = 750 || 751
> [Task-2] Execution Completed. Response Time : 251
> Task[1] --> sending data to --> Task[2]
> [Task-3] Released : 500
> [Task-3] Pre-empted for 251 ticks [500 - 751]
> [Task-3] Running with #ticks = 800 || 851
> [Task-3] Running with #ticks = 850 || 851
> [Task-3] Execution Completed. Response Time : 351
> [Task-1] Released : 900
> [Task-1] Running with #ticks = 900 || 1000
> [Task-1] Running with #ticks = 950 || 1000
> [Task-1] Running with #ticks = 1000 || 1000
> [Task-1] Execution Completed. Response Time : 100
> Task[0] --> sending data to --> Task[1]
> [Task-2] Released : 1000
> [Task-2] Running with #ticks = 1000 || 1150
> [Task-2] Running with #ticks = 1050 || 1150
> [Task-2] Running with #ticks = 1100 || 1150
> [Task-2] Running with #ticks = 1150 || 1150
> [Task-2] Execution Completed. Response Time : 150
> Task[1] --> sending data to --> Task[2]
> [Task-3] Released : 1000
> [Task-3] Pre-empted for 150 ticks [1000 - 1150]
> [Task-3] Running with #ticks = 1150 || 1250
> [Task-1] Released : 1200
> [Task-1] Running with #ticks = 1200 || 1300
> [Task-1] Running with #ticks = 1250 || 1300
> [Task-1] Running with #ticks = 1300 || 1300
> [Task-1] Execution Completed. Response Time : 100
> Task[0] --> sending data to --> Task[1]
> [Task-3] Pre-empted for 251 ticks [1199 - 1300]
> [Task-3] Running with #ticks = 1300 || 1351
> [Task-3] Running with #ticks = 1350 || 1351
> [Task-3] Execution Completed. Response Time : 351
> [Task-1] Released : 1500
> [Task-1] Reached Hyper-period !!!
> [Task-2] Released : 1500
> [Task-2] Reached Hyper-period !!!
> [Task-3] Released : 1500
> [Task-3] Reached Hyper-period !!!
> code$
>```

**_Trace Messages:_**
>```
> [INFO]  -> Informational traces
> [MAKE]  -> Build traces
> [ERROR] -> Error traces
> [USAGE] -> Command usage
>```
