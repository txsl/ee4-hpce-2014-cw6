#!/bin/bash


PUZZLES="circuit_sim life matrix_exponent option_explicit string_search";

SCALES="100 200 300 400 500 600 700 800 900 1000";

LOG_LEVEL=2;


echo "Starting performance testing script.";

# http://stackoverflow.com/questions/6482377/bash-shell-script-check-input-argument
if [ -z "$1" ]
  then
    echo "No output directory argument supplied. Exiting.";
    exit 2;
fi

# thanks http://www.cyberciti.biz/tips/linux-unix-pause-command.html
read -p "Ensure this is being run from the 'test' directory. Hit [enter] to continue..."

mkdir -p $1

for puz in $PUZZLES; do
    for scale in $SCALES; do
        echo "Running $puz $scale";
        fname="${puz}_${scale}.csv";

        ../bin/run_puzzle $puz $scale $LOG_LEVEL &> $1/$fname;
    done
done

