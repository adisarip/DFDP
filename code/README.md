
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
 7. From there on we will perform some tests and try to make use of the FNR lengths **(to be continued)**

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
> │   │   ├── dfdptask.c
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
>```

**_Sample Execution Run:_**
>- Use the wrapper script **run.py** to build and execute the binaries.
> **./run.py <input_task_data_file\>**
>
>```
>```

**_Trace Messages:_**
>```
> [INFO]  -> Informational traces
> [MAKE]  -> Build traces
> [ERROR] -> Error traces
> [USAGE] -> Command usage
>```
