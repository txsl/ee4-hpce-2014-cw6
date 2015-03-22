# Adapted from HPCE-CW3
import sys

# Need to have numpy, pandas and matplotlib installed for this script to work.
# aka: `pip install pandas numpy matplotlib`
import matplotlib.pyplot as plt
import numpy as np

from lib import SCALE_MAPPING, get_ref_time, get_test_time_chunk, test_passed, get_test_time

ref_dir = sys.argv[1]
test_dir = sys.argv[2]


# Not currently the neatest of files..
methods = ["string_search"]
# methods = ["life", "median_bits", "option_explicit"]
opencl = ["life"]


# CHUNK_SIZES = [2, 16, 128, 512, 1024, 4096, 16384]

for m in methods:
    legend = []
    size = []

    # results = dict.fromkeys(CHUNK_SIZES, list())
    # scales = dict.fromkeys(CHUNK_SIZES, list())

    results = {}
    scales = {}
    
    # for c in CHUNK_SIZES:
    #     results[c] = []
    #     scales[c] = []

    results['ref'] = []
    scales['ref'] = []
    
    results['act'] = []
    scales['act'] = []

    if m in opencl:
        results['cl'] = []
        scales['cl'] = []

    for size in SCALE_MAPPING[m]:

        time = get_ref_time(m, size, ref_dir)
        # print time
        
        if time is not None:
            results['ref'].append(time)
            scales['ref'].append(size)

        if test_passed("{}/compare_output_{}_{}.log". format(test_dir, m, size)):
            time = get_test_time("{}/non_ref_output_{}_{}.log".format(test_dir, m, size))
            # time = get_test_time_chunk(m, size, c, test_dir)
                
            if time is not None:
                results['act'].append(time)
                scales['act'].append(size)

        if m in opencl:
            time = get_test_time_chunk(m, size, None, test_dir, True)
            results['cl'].append(time)
            scales['cl'].append(size)


    plt.plot(scales['act'], results['act'], marker='o')
    legend.append('Optimised')

    plt.plot(scales['ref'], results['ref'], marker='o')

    legend.append("Reference Code")    

    if m in opencl:
        plt.plot(scales['cl'], results['cl'], marker='o')
        legend.append("OpenCL")

    plt.yscale('log')
    plt.xscale('log')

    plt.title(m)
    plt.xlabel('Problem Size')
    plt.ylabel('Execution time (seconds)')

    plt.legend(legend, loc='upper left')
    plt.show()

