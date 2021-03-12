
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
        cout << "[USAGE] ./bin/fpds <taskset_input_file>" << endl;
        return 0;
    }

    string input_file  = argv[1];
    FPDS xFpds(input_file);

    cout << "\nInitial Order of Taskset : " << flush;
    xFpds.displayPriorityOrder();
    cout << endl;

    bool isSchedulable = xFpds.computeOptimalPriorityOrder();

    if (isSchedulable)
    {
        cout << "\n[FPDS] The given taskset is SCHEDULABLE using FPDS\n" << endl;
        cout << "Final Schedulable Order of Taskset : " << flush;
        xFpds.displayPriorityOrder();
    }
    else
    {
        cout << "\n[FPDS] The given taskset is NOT-SCHEDULABLE using FPDS\n" << endl;
    }


    return 0;
}
