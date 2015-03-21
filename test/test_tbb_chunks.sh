#!/bin/bash

CHUNK_SIZES="2 16 128 512 1024 4096 16384"

# OPENLCL="0"

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

echo "Starting TBB chunk size testing for ${2}"

source common.sh

mkdir -p $2

if [ $puz == "matrix_exponent" ]; then
    log_scales=$MATRIX_EXPONENT_LOG_SCALES
elif [ $puz == "circuit_sim" ]; then
    log_scales=$CIRCUIT_SIM_LOG_SCALES
elif [ $puz == "life" ]; then
    log_scales=$LIFE_LOG_SCALES
    OPENCL="1"
else
    log_scales=$LARGE_LOG_SCALES
fi

export HPCE_CL=0

for chunk in $CHUNK_SIZES; do
    export HPCE_CHUNKSIZE_K=$chunk;

    for scale in $log_scales; do
        fname="${puz}_${scale}"
        output_fname="${puz}_${scale}_K_${chunk}"

        echo "Creating test output for ${puz} at scale ${scale}, with chunk size ${chunk}, and comparing with reference"

        ../bin/compare_puzzle_output "${1}/${fname}_ref_output" <( ../bin/execute_puzzle 0 $LOG_LEVEL < "${1}/${fname}_input" \
                                 2> "${2}/non_ref_output_${output_fname}.log" ) $LOG_LEVEL 2> "${2}/compare_output_${output_fname}.log"
    done
done

# And the OpenCL loop
if [ ! -z "$OPENCL" ]
    then

    export HPCE_CL=1
    
    for scale in $log_scales; do
        fname="${puz}_${scale}"
        output_fname="${puz}_${scale}_CL"

        echo "Creating test output for ${puz} at scale ${scale}, using OpenCL, and comparing with reference"

        ../bin/compare_puzzle_output "${1}/${fname}_ref_output" <( ../bin/execute_puzzle 0 $LOG_LEVEL < "${1}/${fname}_input" \
                                 2> "${2}/non_ref_output_${output_fname}.log" ) $LOG_LEVEL 2> "${2}/compare_output_${output_fname}.log"
    done
fi