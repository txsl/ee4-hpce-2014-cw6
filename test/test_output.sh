#!/bin/bash


echo "Generating outputs (non reference), and comparing results with the ref out"

source common.sh

# http://stackoverflow.com/questions/6482377/bash-shell-script-check-input-argument
if [ -z "$1" ]
  then
    echo "Input directory for ref comparisons needed"
    exit 2
fi

if [ -z "$2" ]
    then
    echo "Output directory for log files must be specified" 
    exit 2
fi

if [ ! -z "$3" ]
    then
    echo "Specific puzzle ${3} being tested (only)"
    PUZZLES=$3
fi

# should make both levels of directories if necessary
mkdir -p $2


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

        echo "Running ${puz} at scale ${scale}, and comparing with reference"
        ../bin/compare_puzzle_output "${1}/${fname}_ref_output" <( ../bin/execute_puzzle 0 $LOG_LEVEL < "${1}/${fname}_input" 2> "${2}/non_ref_output_${fname}.log" ) $LOG_LEVEL 2> "${2}/compare_output_${fname}.log"
    done

done
