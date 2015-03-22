#!/bin/bash


OPENCL="0"
TBB="1"
NORM="0"

if [ -z "$1" ]
  then
    echo "Input directory argument supplied. Exiting."
    exit 2
fi

if [ -z "$2" ]
    then
    echo "Output directory for log files must be specified" 
    exit 2
fi

if [ ! -z "$3" ]
    then
    echo "Specific puzzle ${3} being tested"
    puz=$3
fi

source common.sh

mkdir -p $2

if [ $puz == "matrix_exponent" ]; then
    log_scales=$MATRIX_EXPONENT_LOG_SCALES
    OPENCL="1"
    TBB="0"
    NORM="1"

elif [ $puz == "circuit_sim" ]; then
    log_scales=$CIRCUIT_SIM_LOG_SCALES

elif [ $puz == "life" ]; then
    log_scales=$LIFE_LOG_SCALES
    OPENCL="1"

elif [ $puz == "string_search" ]; then
    CHUNK_SIZES=$STRING_CHUNK_SIZES
    log_scales=$LARGE_LOG_SCALES

else
    log_scales=$LARGE_LOG_SCALES
    TBB="1"
fi

export HPCE_CL_ENABLE=0

if [ ! "$TBB" -eq 0 ]
    then
    for chunk in $CHUNK_SIZES; do
        export HPCE_CHUNKSIZE_K=$chunk;

        for scale in $log_scales; do
            fname="${puz}_${scale}"
            output_fname="${puz}_${scale}_K_${chunk}"

            echo "Running user code output for ${puz} at scale ${scale}, with chunk size ${chunk}, and comparing with reference"

            ../bin/compare_puzzle_output "${1}/${fname}_ref_output" <( ../bin/execute_puzzle 0 $LOG_LEVEL < "${1}/${fname}_input" \
                                     2> "${2}/non_ref_output_${output_fname}.log" ) $LOG_LEVEL 2> "${2}/compare_output_${output_fname}.log"
        done
    done
fi

# And the OpenCL loop
if [ ! "$OPENCL" -eq 0 ]
    then

    export HPCE_CL_ENABLE=1
    
    for scale in $log_scales; do
        fname="${puz}_${scale}"
        output_fname="${puz}_${scale}_CL"

        echo "Running user code output for ${puz} at scale ${scale}, using OpenCL, and comparing with reference"

        ../bin/compare_puzzle_output "${1}/${fname}_ref_output" <( ../bin/execute_puzzle 0 $LOG_LEVEL < "${1}/${fname}_input" \
                                 2> "${2}/non_ref_output_${output_fname}.log" ) $LOG_LEVEL 2> "${2}/compare_output_${output_fname}.log"
    done
fi

# And the 'normal' loop - using neither TBB or OpenCL (used in matrix exponent)™
if [ ! "$NORM" -eq 0 ]
    then

    export HPCE_CL_ENABLE=0
    
    for scale in $log_scales; do
        fname="${puz}_${scale}"
        output_fname="${puz}_${scale}"

        echo "Running user code output for ${puz} at scale ${scale}, and comparing with reference"

        ../bin/compare_puzzle_output "${1}/${fname}_ref_output" <( ../bin/execute_puzzle 0 $LOG_LEVEL < "${1}/${fname}_input" \
                                 2> "${2}/non_ref_output_${output_fname}.log" ) $LOG_LEVEL 2> "${2}/compare_output_${output_fname}.log"
    done
fi


twitter set "@t8lim: work on `hostname` now finished on ${puz} at `date`"
