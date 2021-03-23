

// Imlementing Fixed-Priority Scheduling with Deferred Premption
// by computing the final non-premptive region for each task
// in the level-i priority active period and priority assignment.

#ifndef FPDS_H
#define FPDS_H

#include <map>
#include <vector>
#include <string>
using namespace std;

struct Task
{
    int id;
    int period;
    int execution_time;
    int deadline;
};

class FPDS
{
  public:
    enum ExecutionMode
    {
        DisableLogging = 0, // default
        EnableLogging  = 1  // set when executed in silent mode
    };

    FPDS(string file, FPDS::ExecutionMode mode = FPDS::DisableLogging);
    ~FPDS();

    // Compute the FNR length and Assign Priorities to Taskset
    bool computeOptimalPriorityOrder();

    // Display the Priority Order
    void displayPriorityOrder();

    // protected
  protected:
    // Input data utilities
    vector<string> pSplit(const string& s, char delimiter);

    // Read the task data set and create the list of task objects
    void pReadTaskSet();

    // Compute and set the Hyper period of the given taskset
    void pSetHyperPeriodLength();

    // FNR-PA Healper functions
    // Get the blocking period from the lower priority tasks
    int pGetBlockingPeriod(int level);

    // Compute the Level-i Active Period for a given task at level 'i'
    // For ever task 'i', we assume all the unassigned tasks, which are not yet part of the
    // optimal priotiy order, all higher priority than Task 'i' and recusively compute the level-i
    // active period. There might be cases where the recursive computation donot converge. We handle
    // with proper failsafe mechanism by thresholding it with HyperPeriod.
    int pGetPriorityLevelActivePeriod(int level);

    // Get the schedulable Task with minimum FNR length
    //
    int pGetMinFNRTask(int index, int level, int& minFNRValue);

    // compute the set of local maxima points of the Minimum execution time pieces linear function
    // The list will have the exact times where the highr priority tasks will be released in the
    // current execution period, before deadline.
    vector<int> pGetHighPoints(int g, int index, int level, int activePeriod);

    // data
  private:
    FPDS::ExecutionMode mMode;
    string iFile;
    int mTaskCount;
    int mHyperPeriodLength;
    vector<Task> mTaskSet;
    map<int, int> mFinalFNRLength; // map <level, FNR>
};

#endif // FPDS_H
