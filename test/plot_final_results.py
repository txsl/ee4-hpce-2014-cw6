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
# methods = ["string_search"]
methods = ["life", "median_bits", "option_explicit", "circuit_sim", "matrix_exponent"]
# opencl = ["life"]


# CHUNK_SIZES = [2, 16, 128, 512, 1024, 4096, 16384]

for m in methods:
    legend = []
    size = []

    results = {}
    scales = {}

    speedup = []
    speedup_scale = []

    results['ref'] = []
    scales['ref'] = []
    
    results['act'] = []
    scales['act'] = []


    for size in SCALE_MAPPING[m]:

        time = get_ref_time(m, size, ref_dir)
        
        if time is not None:
            results['ref'].append(time)
            scales['ref'].append(size)

        if test_passed("{}/compare_output_{}_{}.log". format(test_dir, m, size)):
            time = get_test_time("{}/non_ref_output_{}_{}.log".format(test_dir, m, size))
            # time = get_test_time_chunk(m, size, c, test_dir)
                
            if time is not None:
                results['act'].append(time)
                scales['act'].append(size)

                if time != 0 and results['ref'][-1] != 0:
                    speedup.append(results['ref'][-1]/time)
                    speedup_scale.append(size)


    fig, ax1 = plt.subplots()

    ax1.plot(scales['act'], results['act'], marker='o')
    legend.append('Optimised')

    ax1.plot(scales['ref'], results['ref'], marker='o')
    legend.append("Reference Code")

    # ax1.set_ylim(0.001, results['ref'][-1])

    ax1.set_yscale('log')
    ax1.set_xscale('log')


    plt.title(m)
    ax1.set_xlabel('Problem Size')
    ax1.set_xlim(left=10)
    ax1.set_ylabel('Execution time (seconds)')

    ax2 = ax1.twinx()
    ax2.plot(speedup_scale, speedup, color='r')
    ax2.set_ylabel("Speed our_time/ref_time", color='r')
    # ax2.set_ylim(bottom=0)
    ax2.set_xscale('log')
    ax2.set_xlim(left=10)

    ax1.legend(legend, loc='upper left')
    plt.show()

