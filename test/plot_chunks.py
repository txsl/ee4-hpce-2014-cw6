# Adapted from HPCE-CW3
import sys

# Need to have numpy, pandas and matplotlib installed for this script to work.
# aka: `pip install pandas numpy matplotlib`
import matplotlib.pyplot as plt
import numpy as np

from lib import SCALE_MAPPING, get_ref_time, get_test_time_chunk

ref_dir = sys.argv[1]
test_dir = sys.argv[2]

# Not currently the neatest of files..

methods = ["life", "median_bits", "option_explicit"]
opencl = ["life"]


CHUNK_SIZES = [2, 16, 128, 512, 1024, 4096, 16384]

for m in methods:
    legend = []
    size = []
    # time = []
    # test_output = []
    # results = dict.fromkeys(CHUNK_SIZES, [])
    results = {}
    
    for c in CHUNK_SIZES:
        results[c] = []

    results['ref'] = []

    if m in opencl:
        results['cl'] = []

    for size in SCALE_MAPPING[m]:
        results['ref'].append(get_ref_time(m, size, ref_dir))

        for c in CHUNK_SIZES:
            # print c, get_test_time_chunk(m, size, c, test_dir)
            # print len(results)
            # print c, results[c], type(results[c]), type(results)
            results[c].append(get_test_time_chunk(m, size, c, test_dir))

        if m in opencl:
            results['cl'].append(get_test_time_chunk(m, size, None, test_dir, True))
    # for 

    # for s in sizes:

    # print results
       
    #     else:
    #         size.append(s)
    #         ref_output.append(ref_time)
    #         print m, s, ref_time/60
    #         test_output.append(test_time)

    # print 'size:', size, len(size)
    # print 'ref_output:', ref_output, len(ref_output)
    # print 'test_output:', test_output, len(test_output)
    for c in CHUNK_SIZES:
        print  c, results[c]
        # print 'scale', SCALE_MAPPING[m]
        plt.plot(SCALE_MAPPING[m], results[c], marker='o')
        legend.append("Chunksize K={}".format(c))

    plt.plot(SCALE_MAPPING[m], results['ref'], marker='o')
    print 'ref', results['ref']
    legend.append("Reference Code")    

    if m in opencl:
        print 'cl', results['cl']
        plt.plot(SCALE_MAPPING[m], results['cl'], marker='o')
        legend.append("OpenCL")

    # plt.plot(size, test_output)
    # legend.append("Test output")

    plt.yscale('log')
    plt.xscale('log')

    plt.title(m)
    plt.xlabel('Problem Size')
    plt.ylabel('Execution time (seconds)')

    plt.legend(legend, loc='lower right')
    plt.show()

