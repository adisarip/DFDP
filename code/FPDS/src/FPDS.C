

#include "FPDS.H"
#include <cmath>
#include <climits>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

FPDS::FPDS(string file, FPDS::ExecutionMode mode)
:mMode(mode)
,iFile(file)
{
    pReadTaskSet();
    mTaskCount = (int)mTaskSet.size();
    mFinalFNRLength[mTaskCount-1] = 0;
    pSetHyperPeriodLength();
}

FPDS::~FPDS()
{
}

void FPDS::displayPriorityOrder()
{
    int size  = mTaskSet.size();
    for (int i=0; i < size; i++)
    {
        if(i == 0) cout << mTaskSet[i].id;
        else cout << "," << mTaskSet[i].id;
    }
    cout << endl;
}

void FPDS::displayPriorityOrderWithFNRLengths()
{
    int size  = mTaskSet.size();
    cout << size << " ";
    for (int i=0; i < size; i++)
    {
        if(i == 0) cout << mTaskSet[i].id << ":" << mFinalFNRLength[i];
        else cout << "," << mTaskSet[i].id << ":" << mFinalFNRLength[i];
    }
    cout << endl;
}

bool FPDS::computeOptimalPriorityOrder()
{
    bool isSchedulable = false;
    int minFNRValue = INT_MAX;
    int taskIndex = INT_MAX;
    bool isLog = (mMode == FPDS::EnableLogging) ? true : false;

    for (int k=mTaskCount-1; k>=0; k--)
    {
        if (isLog) cout << "=============================================" << endl;
        if (isLog) cout << "[FPDS] Running FNR-PA on level-" << k << endl;
        if (isLog) cout << "=============================================" << endl;

        for (int i=k; i>=0; i--)
        {
            if (isLog) cout << "[FPDS] Computing FNR Length for Task-" << i << endl;
            int currFNRValue = pGetMinFNRTask(i, k, minFNRValue);
            if (currFNRValue <= mTaskSet[i].execution_time && currFNRValue > 0)
            {
                isSchedulable = true;
                if (minFNRValue > currFNRValue)
                {
                    minFNRValue = currFNRValue;
                    taskIndex = i;
                }
            }
            if (currFNRValue < 0)
            {
                isSchedulable = false;
                break;
            }
            if (isLog)
            {
                if ( i!=0) cout << "---------------------------------------------" << endl;
                else cout << endl;
            }

        }
        if (isSchedulable)
        {
            mFinalFNRLength[k] = minFNRValue;
            if (taskIndex < k)
            {
                // perform the transformation
                Task task = mTaskSet[taskIndex];
                for (int x = taskIndex; x<k; x++)
                {
                    mTaskSet[x] = mTaskSet[x+1];
                }
                mTaskSet[k] = task;
            }
        }
        else
        {
            break;
        }
    }

    if (isSchedulable)
    {
        for (auto f : mFinalFNRLength)
        {
            if (isLog) cout << "[FPDS] Final non-pre-emptive region at Level-" << f.first << " = " << f.second << endl;
        }
    }
    return isSchedulable;
}

// protected member definitions

vector<string> FPDS::pSplit(const string& s, char delimiter)
{
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

void FPDS::pReadTaskSet()
{
    ifstream fin;
    string line;
    vector<string> dataset;

    fin.open(iFile, ios::in);
    while (getline(fin, line))
    {
        if (line[0] == '#')
        {
            continue; // skip comments
        }
        else
        {
            dataset.push_back(line);
        }
    }
    fin.close();

    for (string s : dataset)
    {
        vector<string> tokens = pSplit(s, ',');
        Task t;
        t.id = stoi(tokens.at(0));
        t.period = stoi(tokens.at(1));
        t.execution_time = stoi(tokens.at(2));
        t.deadline = stoi(tokens.at(3));
        mTaskSet.push_back(t);
    }
}

int FPDS::pGetMinFNRTask(int index, int level, int& minFNRValue)
{
    bool isLog = (mMode == FPDS::EnableLogging) ? true : false;
    float activePeriod = pGetPriorityLevelActivePeriod(level);
    if (activePeriod < 0)
    {
        return -1;
    }
    if (isLog) cout << "[FPDS] Priority level-" << level << " active period = " << activePeriod << endl;
    float nJobs = ceil(activePeriod / mTaskSet[index].period);
    vector<int> maxFNRLengths;
    for (int g=0; g<nJobs; g++)
    {
        vector<int> highPoints = pGetHighPoints(g, index, level, activePeriod);
        vector<float> minExecTimes;
        for (int t : highPoints)
        {
            int blockingPeriod = pGetBlockingPeriod(level);
            float minExecTime = t - blockingPeriod - g * mTaskSet[index].execution_time;
            for (int i=0; i<=level; i++)
            {
                if (i == index) continue;
                minExecTime -= (floor(t / mTaskSet[i].period) + 1) * mTaskSet[i].execution_time;
            }
            int remExecTime = mTaskSet[index].execution_time - minExecTime;
            int timeToDeadline = g * mTaskSet[index].period + mTaskSet[index].deadline - t;
            if (minExecTime >=0 && remExecTime <= timeToDeadline)
            {
                minExecTimes.push_back(minExecTime);
            }
        }
        int maxOfMinExecTimes;
        if (minExecTimes.size() == 0)
        {
            maxOfMinExecTimes = -1;
        }
        else
        {
            maxOfMinExecTimes = *max_element(minExecTimes.begin(), minExecTimes.end());
        }
        if (isLog) cout << "[FPDS] Max Execution time = " << maxOfMinExecTimes << endl;
        int maxFNRLength = max(mTaskSet[index].execution_time - maxOfMinExecTimes, 1);
        if (isLog) cout << "[FPDS] For Job g = " << g << " max FNR length = " << maxFNRLength << endl;
        maxFNRLengths.push_back(maxFNRLength);
    }

    return *max_element(maxFNRLengths.begin(), maxFNRLengths.end());
}

vector<int> FPDS::pGetHighPoints(int g, int index, int level, int activePeriod)
{
    bool isLog = (mMode == FPDS::EnableLogging) ? true : false;
    vector<int> hPoints;
    int gMinValue = g * mTaskSet[index].period;
    int gMaxValue = g * mTaskSet[index].period + mTaskSet[index].deadline - 1;
    for (int i=0; i<=level; i++)
    {
        if (i == index) continue;
        for (int x=0; x * mTaskSet[i].period <= activePeriod; x++)
        {
            if ((x * mTaskSet[i].period - 1) >= gMinValue &&
                (x * mTaskSet[i].period - 1) <= gMaxValue)
            {
                hPoints.push_back(x * mTaskSet[i].period - 1);
            }
        }
    }
    hPoints.push_back(gMaxValue);
    if (isLog) cout << "[FPDS] Local maxima occur at -> {" << flush;
    for (auto p : hPoints)
    {
        if (isLog) cout << p << "," << flush;
    }
    if (isLog) cout << "}" << endl;
    return hPoints;
}

int FPDS::pGetPriorityLevelActivePeriod(int level)
{
    bool isLog = (mMode == FPDS::EnableLogging) ? true : false;
    int blockingPeriod = pGetBlockingPeriod(level);
    float currActivePeriod = mTaskSet[0].execution_time;
    float prevActivePeriod = 0;
    do
    {
        prevActivePeriod = currActivePeriod;
        currActivePeriod = blockingPeriod;
        for (int i=0; i<=level; i++)
        {
            currActivePeriod += ceil(prevActivePeriod / mTaskSet[i].period) * mTaskSet[i].execution_time;
        }
    } while (currActivePeriod != prevActivePeriod && currActivePeriod <= mHyperPeriodLength);
    if (currActivePeriod > mHyperPeriodLength)
    {
        if (isLog) cout << "[FPDS] Level-i Active Period (" << currActivePeriod << ") is > Hyper Period (" << mHyperPeriodLength << ")" << endl;
        if (isLog) cout << "[FPDS] Level-i Active Period is Not Converging" << endl;
        currActivePeriod = -1;
    }
    return currActivePeriod;
}

int FPDS::pGetBlockingPeriod(int level)
{
    return mFinalFNRLength[level];
}

void FPDS::pSetHyperPeriodLength()
{
    int LCM = mTaskSet[0].period;
    int p = 1;
    for (int i=1; i<mTaskCount; i++)
    {
        p = mTaskSet[i].period;
        LCM = lcm(LCM,p);
    }
    mHyperPeriodLength = LCM;
}
