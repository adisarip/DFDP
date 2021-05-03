#!/usr/bin/env bash

CURRENT_DIR=$PWD
FPDS_SRC_PATH="${CURRENT_DIR}/FPDS/src/"
RTOS_SRC_PATH="${CURRENT_DIR}/FreeRTOSPosix/"
CSCOPE_DB_FILE='dfdp_cscope.files'
CSCOPE_DB_DIR="${CURRENT_DIR}/cscope_db/"

cd $CSCOPE_DB_DIR
rm $CSCOPE_DB_FILE
rm cscope.out cscope.in.out cscope.po.out

echo "Collecting source files from the project ..."
find $FPDS_SRC_PATH -name "*.C" -o -name "*.H" -o -name "*.c" -o -name "*.h" -o -name "*.cpp" > $CSCOPE_DB_FILE
find $RTOS_SRC_PATH -name "*.C" -o -name "*.H" -o -name "*.c" -o -name "*.h" >> $CSCOPE_DB_FILE

echo "Creating cscope database ..."
cscope -C -q -R -b -i $CSCOPE_DB_FILE 2>/dev/null

# go back to the working directory
cd $CURRENT_DIR

