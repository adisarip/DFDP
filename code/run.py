#!/usr/bin/env python3
# A Python wrapper script to work with both FPDS and Data Freshness binaries.

import os
import sys

FPDS_DIR = "/FPDS"
FPDS_BIN = "/FPDS/bin/fpds"

RTOS_DIR = "/FreeRTOSPosix"
RTOS_BIN = "/FreeRTOSPosix/simrtos"

#def run_data_freshness(fpds_priority_order):

def run_fpds(data_file):
    run_fpds_cmd = os.getcwd() + FPDS_BIN + " " + data_file
    #print(run_fpds_cmd)
    fpds_out_stream = os.popen(run_fpds_cmd)
    fpds_output = [line.rstrip() for line in fpds_out_stream.readlines()]
    fpds_priority_order = ""
    if (fpds_output[0] == 'SCHEDULABLE'):
        fpds_priority_order = fpds_output[1]
        #print(fpds_priority_order)
    else:
        print(fpds_output[0])
    return fpds_priority_order

def run_data_freshness(priority_order, data_file):
    run_data_fresh_cmd = os.getcwd() + RTOS_BIN + " " + priority_order + " " + data_file
    print("[INFO] Simulating Data-Freshness on FreeRTOS")
    os.system(run_data_fresh_cmd)

def main(data_file):
    home_dir = os.getcwd()
    # build FPDS binaries
    print("[INFO] Building FPDS binaries ...")
    run_fpds_build_cmd = "cd " + home_dir + FPDS_DIR + "; make; cd " + home_dir
    os.system(run_fpds_build_cmd)

    # build RTOS binaries
    print("[INFO] Building RTOS simulator binaries ...")
    run_rtos_build_cmd = "cd " + home_dir + RTOS_DIR + "; make; cd " + home_dir
    os.system(run_rtos_build_cmd)

    # Compute the priority order of the task-set using FPDS
    priority_order = run_fpds(data_file)

    # Compute the data freshness based on the priority order.
    freshness_data = run_data_freshness(priority_order, data_file)
    #print(freshness_data)

# script starts here - main
if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("[ERROR] Input file missing\n")
        sys.exit()
    test_data_file = sys.argv[1]
    main(test_data_file)
