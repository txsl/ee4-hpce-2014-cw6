#!/bin/bash

echo "Starting reference script - Generates input and reference output, with timings"

source common.sh

# http://stackoverflow.com/questions/6482377/bash-shell-script-check-input-argument
if [ -z "$1" ]
  then
    echo "No output directory argument supplied. Exiting."
    exit 2
fi

if [ ! -z "$2" ]
    then
    echo "Specific puzzle ${2} being tested (only)"
    PUZZLES=$3
fi

# thanks http://www.cyberciti.biz/tips/linux-unix-pause-command.html
read -p "Ensure this is being run from the 'test' directory. Hit [enter] to continue..."

# should make both levels of directories if necessary
mkdir -p $1/log

for puz in $PUZZLES; do

    if [ $puz == "matrix_exponent" ]; then
        log_scales=$MATRIX_EXPONENT_LOG_SCALES
    elif [ $puz == "circuit_sim" ]; then
        log_scales=$CIRCUIT_SIM_LOG_SCALES
    elif [ $puz == "life" ]; then
        log_scales=$LIFE_LOG_SCALES
    else
        log_scales=$LARGE_LOG_SCALES
    fi

    for scale in $log_scales; do
        # echo "Running $puz $scale"
        fname="${puz}_${scale}"

        echo "Creating puzzle input for ${puz} at scale ${scale}"
        ../bin/create_puzzle_input $puz $scale $LOG_LEVEL > "${1}/${fname}_input" 2> "${1}/log/create_${fname}.log"

        echo "Creating ref output for ${puz} at scale ${scale}"
        ../bin/execute_puzzle 1 $LOG_LEVEL < "${1}/${fname}_input" > "${1}/${fname}_ref_output" 2> "${1}/log/ref_output_${fname}.log"
    done

done

