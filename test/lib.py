import csv

LARGE_LOG_SCALES = [1, 5, 10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000]
CIRCUIT_SIM_LOG_SCALES = [1, 5, 10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000]
MATRIX_EXPONENT_LOG_SCALES = [1, 5, 10, 50, 100, 500, 1000]
LIFE_LOG_SCALES = [1, 5, 10, 50, 100, 500, 1000, 5000]

SCALE_MAPPING = {
                    'life': LIFE_LOG_SCALES, 'option_explicit': LARGE_LOG_SCALES, 'median_bits': LARGE_LOG_SCALES,
                    'string_search': LARGE_LOG_SCALES,
                    }

def get_ref_time(puzzle, size, dirname):

    name = "{0}/log/ref_output_{1}_{2}.log".format(dirname, puzzle, str(size))

    with open(name, "r") as csvfile:
        reader = csv.reader(csvfile)
        good_output = False

        for idx, row in enumerate(reader):
            if idx == 0:
                continue

            if row[3] == ' Begin reference':
                start_test = float(row[1])
            
            elif row[3] == ' Finished reference':
                ref_time = float(row[1]) - start_test

    # Correction to floats 
    if ref_time < 0.01:
        ref_time = 0.00

    return ref_time         

def test_passed(filename):
    
    try:
        with open(filename, "r") as csvfile:
            reader = csv.reader(csvfile)
            for idx, row in enumerate(reader):

                if idx == 0:
                    continue

                if row[3] == ' Outputs are equal.':
                    return True
    except IndexError:
        pass

    print "TEST DID NOT PASS! {}".format(filename)
    return False

def get_test_time(filename):

    with open(filename, "r") as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            
            if row[3] == ' Begin execution':
                start_test = float(row[1])

            elif row[3] == ' Finished execution':
                ref_time = float(row[1]) - start_test

    # Correction to floats 
    if ref_time < 0.01:
        ref_time = 0.00

    return ref_time

def get_test_time_chunk(puzzle, size, chunksize, dirname, cl=False):

    if cl:
        core_str = "{}_{}_CL".format(puzzle, size)
    else:
        core_str = "{}_{}_K_{}".format(puzzle, size, chunksize)
    
    if test_passed("{}/compare_output_{}.log".format(dirname, core_str)):
        return get_test_time("{}/non_ref_output_{}.log".format(dirname, core_str))

    else:
        return None

