# Adapted from HPCE-CW3
import sys, csv

# Need to have numpy, pandas and matplotlib installed for this script to work.
# aka: `pip install pandas numpy matplotlib`
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


dirname = sys.argv[1]

# Not currently the neatest of files..

methods = ["circuit_sim", "life", "matrix_exponent", "option_explicit", "string_search"]
sizes = range(100, 1100, 100)




for m in methods:
    legend = []
    size = []
    ref_output = []
    test_output = []
    
    for s in sizes:

        name = "{0}/{1}_{2}.csv".format(dirname, m, s)

        with open(name, "r") as csvfile:
            reader = csv.reader(csvfile)
            good_output = False

            for idx, row in enumerate(reader):
                if idx == 0:
                    continue

                if row[3] == ' Executing puzzle':
                    start_test = float(row[1])
                
                elif row[3] == ' Executing reference':
                    start_ref_time = float(row[1])
                    test_time = start_ref_time - start_test
                
                elif row[3] == ' Checking output':
                    ref_time = float(row[1]) - start_ref_time

                elif row[3] == ' Output is correct':
                    good_output = True

        if not good_output:
            print "TEST DID NOT PASS! Puzzle: {0} Size: {1}".format(m, s)
       
        else:
            size.append(s)
            ref_output.append(ref_time)
            test_output.append(test_time)

    # print 'size:', size, len(size)
    # print 'ref_output:', ref_output, len(ref_output)
    # print 'test_output:', test_output, len(test_output)

    plt.plot(size, ref_output)
    legend.append("Reference output")

    plt.plot(size, test_output)
    legend.append("Test output")

    plt.yscale('log')
    plt.xscale('log')

    plt.title(m)
    plt.xlabel('Problem Size')
    plt.ylabel('Execution time (seconds)')

    plt.legend(legend, loc='upper left')
    plt.show()

