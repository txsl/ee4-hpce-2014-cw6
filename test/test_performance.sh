#!/bin/bash


import common.sh

echo "Starting performance testing script.";

# http://stackoverflow.com/questions/6482377/bash-shell-script-check-input-argument
if [ -z "$1" ]
  then
    echo "No output directory argument supplied. Exiting.";
    exit 2;
fi

if [ -z "$2" ]
  then
    echo "Specific puzzle ${2} selected";
    exit 2;
fi


# thanks http://www.cyberciti.biz/tips/linux-unix-pause-command.html
read -p "Ensure this is being run from the 'test' directory. Hit [enter] to continue..."

mkdir -p $1

for puz in $PUZZLES; do

    if [ $puz == "matrix_exponent" ]; then
        log_scales=$MATRIX_EXPONENT_LOG_SCALES
        
    elif [ $puz == "circuit_sim" ]; then
        log_scales=$CIRCUIT_SIM_LOG_SCALES

    elif [ $puz == "life" ]; then
        log_scales=$LIFE_LOG_SCALES

    else
        log_scales=$LARGE_LOG_SCALES
        TBB="1"
    fi

    for scale in $log_scales; do
        echo "Running $puz $scale";
        fname="${puz}_${scale}.csv";

        ../bin/run_puzzle $puz $scale $LOG_LEVEL &> $1/$fname;
    done
done

