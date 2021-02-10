#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define gettid() syscall(__NR_gettid)

#define SCHED_DEADLINE	6

/* XXX use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr		314
#define __NR_sched_getattr		315
#endif

#ifdef __i386__
#define __NR_sched_setattr		351
#define __NR_sched_getattr		352
#endif

#ifdef __arm__
#define __NR_sched_setattr		380
#define __NR_sched_getattr		381
#endif

static volatile int done;
static unsigned num_threads = 3;
struct thread_data* thread_data_array;
static double freshness;
pthread_spinlock_t lock;
static int pshared;
static struct timespec* data_to_be_fresh, *temp_data_to_be_fresh;
static volatile int num_reads = 0;
double avg_bound = 0.0;
static ofstream output("OutputEDF.txt");
static int trial_num = 1;
static int num_schedulable = 0;

double avg_utilization = 0;
double avg_avg_freshness = 0;
double avg_max_staleness = 0;

struct sched_attr {
	__u32 size;

	__u32 sched_policy;
	__u64 sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;

	/* SCHED_DEADLINE (nsec) */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags) {
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

struct thread_data {
    unsigned int id;
    double period;
    double wcet;
    double deadline;
};

void *run_edf(void* data) {
	thread_data this_thread_data = *((thread_data*)data);
	
	struct sched_attr attr;
	int ret;
	unsigned int flags = 0;

	attr.size = sizeof(attr);
	attr.sched_flags = 0;
	attr.sched_nice = 0;
	attr.sched_priority = 0;

	// Find out which thread we are and set up parameters
	attr.sched_policy = SCHED_DEADLINE;
    
    // Get attributes
    attr.sched_runtime = (unsigned)(this_thread_data.wcet * 1000 * 1000);
    attr.sched_period = (unsigned)(this_thread_data.period * 1000 * 1000);
    attr.sched_deadline = (unsigned)(this_thread_data.deadline * 1000 * 1000);

	// Set thread parameters
	ret = sched_setattr(0, &attr, flags);
	if(ret < 0) {
		done = 0;
		perror("sched_setattr");
		cout << this_thread_data.id << " attr: " << attr.sched_runtime << endl << flush;
		cout << this_thread_data.id << " attr: " << attr.sched_period << endl << flush;
		exit(-1);
	}

	// Do our work
	struct timespec now;
    now.tv_sec = now.tv_nsec = 0;
	double avg_freshness = 0.0;
	int misses = 0;
	double miss_percent = 0.0;
	double milliseconds;
	num_reads = 0;
	double max_staleness = 0.0;
	while(!done) {
		if(this_thread_data.id == 1) {
			// Sleep some of period
			usleep((int)(this_thread_data.wcet*.99) * 1000);
			// "Create" data: get time, put into first time slot
			pthread_spin_lock(&lock);
			clock_gettime(CLOCK_MONOTONIC, &data_to_be_fresh[0]);
			pthread_spin_unlock(&lock);
		}
		else if(this_thread_data.id < num_threads) {
			pthread_spin_lock(&lock);
            temp_data_to_be_fresh[this_thread_data.id-1] = data_to_be_fresh[this_thread_data.id-2];
			pthread_spin_unlock(&lock);
			// Sleep some of period
			usleep((int)(this_thread_data.wcet*.99) * 1000);
			// Copy data from first to second time slot
			pthread_spin_lock(&lock);
            data_to_be_fresh[this_thread_data.id-1] = temp_data_to_be_fresh[this_thread_data.id-1];
			pthread_spin_unlock(&lock);
		}
		else if(this_thread_data.id == num_threads) {
			// Read value and compare to current time, compare to freshness
			clock_gettime(CLOCK_MONOTONIC, &now);
			pthread_spin_lock(&lock);
			milliseconds = (((double)now.tv_sec + 1.0e-9*now.tv_nsec) - 
                ((double)data_to_be_fresh[num_threads-2].tv_sec + 1.0e-9*data_to_be_fresh[num_threads-2].tv_nsec)) * 1000;
			pthread_spin_unlock(&lock);
			
			if(milliseconds > 10000) goto skip;
			
			if(milliseconds > freshness) {// && milliseconds - freshness <= tC_period) {
				misses++;
				miss_percent += milliseconds - freshness;
				//avg_freshness += milliseconds;
			}
			else {
				avg_freshness += milliseconds;
			}
			num_reads++;
			
			if(milliseconds > max_staleness && milliseconds <= freshness) // && milliseconds < tC_period)
				max_staleness = milliseconds;
			
			skip:
			// Sleep rest of period
			usleep((int)(this_thread_data.wcet*.99) * 1000);
		}
	}
	
	if(this_thread_data.id == num_threads) {
		avg_freshness = avg_freshness / num_reads;
		miss_percent = (double)misses / num_reads * 100;
		avg_max_staleness += max_staleness / freshness;
		printf("Freshness Bound: %f\nAverage Freshess: %f\nAverage Freshness Percent: %f\nMisses: %d\nMiss percent: %f\nRuns: %d\nMax Staleness: %f\n\n", freshness, avg_freshness, avg_freshness / freshness * 100, misses, miss_percent, num_reads, max_staleness);
		avg_avg_freshness += avg_freshness / freshness * 100;
	}

	return NULL;
}

int main(int argc, char** argv) {	
	// Read file
	string line;
	//~ ifstream input("ThreeTaskTrialsQuarter.txt");
    ifstream input("e3s_RM.txt");
	if(!input.is_open()) {
		printf("Could not open file\n");
		return 0;
	}
	else {
		printf("File opened\n");
	}
	
	int ret = pthread_spin_init(&lock, pshared);
	if(ret < 0) {
		printf("Lock creation failed");
		return 0;
	}
	trial_num = 1;
    num_schedulable = 0;
	
	int num_lines = 0;
	input >> num_lines;
    //num_lines = 250;
	
	int i;
	for(i = 1; i <= num_lines; i++) {
		printf("Starting Trial %d...\n", trial_num);
		
		// Get next trial
        input >> num_threads;
        pthread_t* threads = new pthread_t[num_threads];
        thread_data_array = new thread_data[num_threads];
        data_to_be_fresh = new timespec[num_threads];
        temp_data_to_be_fresh = new timespec[num_threads];
        
        double util = 0.0;
        for(unsigned int j = 0; j < num_threads; j++) {
            input >> thread_data_array[j].period;
            input >> thread_data_array[j].wcet;
            // If needed to convert to milliseconds
            thread_data_array[j].period *= 1000;
            thread_data_array[j].wcet *= 1000;
            
            util += thread_data_array[j].wcet / thread_data_array[j].period;
            thread_data_array[j].deadline = thread_data_array[j].period;
            thread_data_array[j].id = j+1;
            
            data_to_be_fresh[j].tv_sec = data_to_be_fresh[j].tv_nsec = 0;
            temp_data_to_be_fresh[j].tv_sec = temp_data_to_be_fresh[j].tv_nsec = 0;
        }
		freshness = thread_data_array[num_threads-1].period;
        thread_data_array[num_threads-1].wcet = 0.1;
        
		if(util > 1.0) {
			printf("Not schedulable, skipping\n");
			trial_num++;
			continue;
		}
		else {
            num_schedulable++;
			printf("Schedulable, util = %f\n", util);
			avg_utilization += util;
			printf("Task 1 period = %f\n", thread_data_array[0].period);
		}
        
		avg_bound += freshness;
		
		// Reset trial variables
		done = 0;
		
		// Create threads
        for(unsigned int j = 0; j < num_threads; j++) {
            pthread_create(&threads[j], NULL, run_edf, (thread_data*)&thread_data_array[j]);
        }
		
		// Each trial is one minute
		sleep(60);
		
		// Signal end of trial, wait for threads to finish
		done = 1;
        for(unsigned int j = 0; j < num_threads; j++) {
            pthread_join(threads[j], NULL);
        }
		
		//printf("Done.\n");
		trial_num++;
        
        free(threads);
        free(thread_data_array);
        free(data_to_be_fresh);
        free(temp_data_to_be_fresh);
	}
	
	avg_bound = avg_bound / num_schedulable;
	avg_utilization = avg_utilization / num_schedulable * 100;
	avg_avg_freshness = avg_avg_freshness / num_schedulable;
	avg_max_staleness = avg_max_staleness / num_schedulable * 100;
	printf("Average Bound: %f\nAvg Util: %f\nAvg Avg Freshness: %f\nAvg Max Staleness: %f\n", avg_bound, avg_utilization, avg_avg_freshness, avg_max_staleness);
	
	printf("Exiting\n");
	input.close();
	output.close();
	return 0;
}
