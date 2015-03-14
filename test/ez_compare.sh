#!/bin/bash

cd "$( dirname "${BASH_SOURCE[0]}" )"/../

if [[ $# != 2 ]]; then
	tput setaf 1; 
	echo
	echo "###############################################"
	echo "#                INVALID USAGE                #"
	echo "###############################################"
	echo
	echo "          \"That was a bit silly of you\""
	echo "                          -Dario Magliocchetti"
	tput setaf 0;
	echo 
	echo
	echo "Usage: "
	echo "    $0 <puzzle name> <scale>"
	echo
	echo 
	exit
fi

tput setaf 2; 
echo
echo "###############################################"
echo "#                CORRECT USAGE                #"
echo "###############################################"
echo
echo "          \"Well done you\""
echo "                          -Dario Magliocchetti"
echo
echo


tput sgr0;

echo "~~~~> Building files"
	make --quiet all
	tput setaf 2; 
	echo "Done"
	echo 
	tput sgr0; 	

echo "~~~~> Running test on --$1-- with scale --$2--"


	./bin/compare_puzzle_output \
		<(./bin/create_puzzle_input $1 $2 0 | ./bin/execute_puzzle 1 0) \
		<(./bin/create_puzzle_input $1 $2 0 | ./bin/execute_puzzle 0 0) 3

	tput setaf 2; 
	echo "Done"
	echo 
	tput sgr0; 	

