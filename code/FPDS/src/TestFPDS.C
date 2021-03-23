
// Test the FPDS Algorithm

#include "FPDS.H"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "[ERROR] Input File Missing." << endl;
        cout << "[USAGE] ./bin/fpds <taskset_input_file> [execution_mode]" << endl;
        cout << "[USAGE] Sample Runs:" << endl;
        cout << "        ./bin/fpds taskset.txt" << endl;
        cout << "        ./bin/fpds taskset.txt -debug" << endl;
        return 0;
    }

    FPDS::ExecutionMode sMode = FPDS::DisableLogging;
    bool isLog = false;
    if (argc == 3 && string(argv[2]) == "-debug")
    {
        // mode of operation supplied
        sMode = FPDS::EnableLogging;
        isLog = true;
    }

    string input_file  = argv[1];
    FPDS xFpds(input_file, sMode);

    if (isLog) cout << "\nInitial Order of Taskset : " << flush;
    if (isLog) xFpds.displayPriorityOrder();
    if (isLog) cout << endl;

    bool isSchedulable = xFpds.computeOptimalPriorityOrder();

    if (isSchedulable)
    {
        if (isLog) cout << "\n[FPDS] The given taskset is SCHEDULABLE using FPDS\n" << endl;
        if (isLog) cout << "Final Schedulable Order of Taskset : " << flush;
        cout << "SCHEDULABLE" << endl;
        xFpds.displayPriorityOrder();
    }
    else
    {
        if (isLog) cout << "\n[FPDS] The given taskset is NOT-SCHEDULABLE using FPDS\n" << endl;
        cout << "NOT-SCHEDULABLE" << endl;
    }


    return 0;
}
