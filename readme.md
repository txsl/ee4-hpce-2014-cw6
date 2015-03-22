# HPCE6 

After working diligently on optimising the given algorithms, we returned our code to the sales and marketing team. As these algorithms are highly commercially sensitive, details of our conversations must remain secret. However, vocabulary used in the meeting room upon presenting our results included "bloody amazing" and "best thing since Jesus". We've seen a 100% market takeover in only 48 hours, and the optimisations have been critical in raising a valuation of £1.2tn. The sales team were more than happy, now earning a healthy commission on sales of these excellent programs. 

Furthermore, the CEO of the company has personally applauded us as voluntary students, and has offered us full time positions as executives. Honoured by the offer, but bound by Peter Cheung to decline, we instead accepted a program license to use the below 6 algorithms without limit. Considering this would otherwise cost a full £15.00, we think the project has been a good experience for us.

We move forward now looking at applying our excellent optimisation strategies at other prestigious software companies such as Kodak or HP.

# Approach 

After understanding each algorithm's functionality, we searched for :
- Potential Algorithm Improvements
- Potential TBB use
- Potential OpenCL use

A number of attempts were made to improve the speed of algorithms (see testing), and **optimised results were hard coded to provide the best possible speed on AWS g2.2xlarge instances** (*this is a competition, after all!*). Each optimised algorithm has code that allows the fine tuning of TBB and OpenCL parameters, and these were used to determine optimum values. In an attempt to save set up time though, these have been removed in our final submission ([this commit](https://github.com/HPCE/hpce_2014_cw6_dm1911_txl11/commit/4307ba0ce965ff887d8055a0118fc2b0c3ded31d) will show insight of what we had previously). Sales people don't know how to use `export` anyhow, so all is good.

Notes on how we came to our conclusions are at the bottom of this readme under "Optimisation Conclusions".

# Disclaimers and Notes

## Disclaimer

All testing has been carried out on Macintosh, Linux (DOC) and AWS machines. For AWS, only a GPU instance with `HPCE-2014-GPU-Image` was tested.

OpenCL requires executables to be run from the project root (i.e. `./bin/run_puzzle` as opposed to `cd bin && ./run_puzzle`). One could also export the relevant environment variable `HPCE_CL_SRC_DIR`.

## Notes

An OpenCL [shared library](https://github.com/HPCE/hpce_2014_cw6_dm1911_txl11/blob/master/provider/opencl_frame.hpp) was created to keep code clean. This is very basic, but fit to serve it's purpose for this coursework. Please note that, as the current way it's been implemented, it only allows for OpenCL to be run on device and platform 0.

This is an easy change to fix, but not required for our purposes / for the given spec.

## Division of labour

Initially, both dm1911 and txl11 spent a couple days to understand and come to terms with the code base. The sales team, acting as expected, were more than displeased at how long this took. With some help from the development team however, the sales and marketing team trusted us a little more.

After looking and understanding the core framework, we spent a bit of time working together on analysing each algorithm as simply as possible. Involved in this discussion was the approach to optimisation (i.e. where are the biggest bottlenecks). The algorithms were then split as follows:

- dm1911 to take `matrix_exponent`, `median_bits` and `circuit_sim`.
- txl11 to take `life`, `option_explicit` and `string_search`.

Further more, we added the following tasks to each others to-do list.

- txl11 to run testing using python and matplotlib. See `Testing` section of this report.
- txl11 to look at creating a Makefile that works across linux and osx.
- dm1911 to create a standard OpenCL library for use across our implementations

# The Puzzles

### Puzzle: Life

`Life` is a classic stencil type operation, with each cell's next state in the grid needing to be computed for each time step. There is a clear loop dependency between each time step, but the calculation of each cell within a given time step can be calculated in parallel. Two parallelisation methods were tested: Threaded Building Blocks Parallel For loop, and OpenCL.

In order for either optimisation to be applied, a modification had to be made. Since `life` looks at whether an individual cell is 'alive' or not, it was originally stored in a Vector of Bools. It is [well known](https://www.google.co.uk/?q=c%2B%2B+vector+bool) that the C++ implementation of a Vector of Bools is [not recommended](http://www.codingstandard.com/rule/17-1-1-do-not-use-stdvector/), and does not guarantee safe modification of values concurrently. Thus it must be converted to another datatype. In this case, they were converted to integers. Modification of the `update` function was also necessary for this to work (although some implicit type conversion on its return value does take place).

The TBB implementation was created using `tbb::parallel_for`, with the outer loop being parallel-ised. In other words, a chunk represents a certain number of columns, and the inner for loop executes across those rows (and these are running in chunked parallel groups).

An OpenCL implementation would potentially run even faster, since it can operate across both dimensions in one operation, since it has plenty of cores. It also works well since we feed in the starting conditions (data about each point), and save the computed next state. By using pointers carefully, we can swap the pointers to each vector of data between iterations, and avoid the need to read the data to and from the GPU until the very end. Only then do we read the data from the GPU's memory back to be saved to the output.

Based on tests run on AWS, we determined that for problem sizes of less than 250, the TBB `parallel_for` method will be used, and for problems larger the OpenCL implementation will be used. OpenCL takes longer to initialise, but for large problem sets pays off with significant gains in execution time.

For problem sizes where TBB is used, the optimum chunk size of 128 was chosen. 

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/final/life.png)

### Puzzle: Matrix Exponent

- Algorithm Optimisation (O(N^3) to O(N^2) for matrix multiplication)
- OpenCL with some clever modifications.

Matrix Exponent was an interesting program to look at. Matrix `A` is created using a randomly generated seed, and this is passed "multiplied" with itself multiple times. A hash is then created using the first item (row 1, column 1) from each intermediate matrix.

Interestingly, the David Thomas Matrix Multiplication method was used to calculate the exponent of each matrix. Though many mathematicians would be appalled by the use of such inprecise methods, the computer scientists in us were delighted to see the number of optimisations that could be done here. Specifically, as each column in the accumulator matrix was similar, the N \* N \* N operation was reduced to N \* N. We then looked at converting this to OpenCL, as this problem consist of many smaller calculations without the need to move much data around. There was little room for parallelisation between each loop (as each loop of the matrix exponent was enviromnedant on the accumulator matrix), so TBB was not considered.

In using OpenCL, buffers are swapped and only the first element is read from the matrix. This is as the `hash` item only consists of the first element of the accumulator matrix at each iteration. As there is an overhead in using OpenCL, only once the matrix is of dimension 250 do we convert to OpenCL.

    for (unsigned i = 2; i < input->steps; i++) {
        kernel.setArg(0, buffCurrVector);
        kernel.setArg(2, buffNextVector);
        queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize); 
        queue.enqueueBarrier();
        //read only the first element straight into hash[i].
        queue.enqueueReadBuffer(buffNextVector, CL_TRUE, 0, sizeof(uint32_t), &hash[i]);
        std::swap(buffCurrVector, buffNextVector);
    }
    
Thought not tested, a further optimisation to look into would have been using TBB on `MatrixExponentPuzzle::MatrixCreate()` to speed up with creation of the `A` file. 

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/final/matrix_exponent.png)

### Puzzle: String Search

String Search is searching for specific DNA patterns in a long string which represents a DNA sequence. For a given string, there is more than one pattern to find. The way in which the search code works is quite specific:

It searches for a pattern match, iterating through each pattern at a given index in the string, looking for to match a pattern. It does this one by one (ie at a given point in the string, it iterates through each pattern looking for a match - see code below). As soon as it finds a match, it stops looking for patterns at that index, and updates the index (of where it is located in the string) to the next location after the matched pattern. If no pattern is found, it increments the index by one (ie moves along by one) and starts looking for a pattern once again.

    // This is the important for loop where each pattern is tested and if a match found, the loop is broken out of
    for (unsigned p = 0; p < pInput->patterns.size(); p++) {
        unsigned len = Matches(data, i, pInput->patterns[p]);
        if (len > 0) {
            log->Log(Log_Debug, [&](std::ostream & dst) {
                dst << "  Found " << pInput->patterns.at(p) << " at offset " << i << ", match=" << data.substr(i, len);
        });
        histogram[p]++;
        i += len - 1;
        break;
    }

If we are lucky and the pattern match occurs for the first pattern in our series, we will avoid unnecessary calculations. On the other end of the spectrum, if the last pattern is the one matched each time, all of the other pattern searches (for different patterns) were futile, and most importantly for us, a waste of time.

The first strategy attempted was to blindly use a TBB parallel for loop for the search for a match. This proved to break the output. The nature of operation (searching through each pattern _until_ a match is found) makes it difficult to run in parallel, since knowing when to stop searching may not occur in the correct order. In hindsight this was a futile endeavour.

The second strategy was to create a vector of size `string_length` \ * `number_of_patterns`. And in a large for loop (which would be easily optimisable through either TBB or perhaps even OpenCL) each position and pattern would be tested. Once this had finished executing, a for loop is run to analyse the correct answers and build up the histogram. Whilst this did work for small values as a test case, it failed for large problem sizes due to the amount of memory needed. For example, a 1000 long string with 31 patterns (based on the function used to generate patterns) would create a vector with 31,000 elements.

The final strategy was to create a vector the length of the number of patterns. The for loop had its break line removed, and instead the output of each call to `Matches` was saved to the relevant location in the vector. This is the part which has been parallelised - it assumes that on average a parallel loop will solve the problem faster than if we had to wait for the last call to `Matches` to find a match. After the for loop had finished, another for loop runs to look for the first match in the vector and then breaks out of the for loop. This worked, and once integrated with TBB's Parallel For was shown to run faster than the reference execution (although a modest speed increase). It's worth noting that the chunk size was made purposely smaller here than others, since there are (in comparison to the other problems) many fewer threads. But for large problem sizes (for example, 1,000,000), this chunk size will need to be made larger.


![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/final/string_search.png)

### Puzzle: Option Explicit
- TBB to calculate initial state
- TBB to calculate future states (inner loop)

In the algorithm used to determine the value of certain derivatives, a fast execution time was vital. A number of parameters are calculated / generated, and then passed though two `for` loops; the fist to calculate the initial states, and the second to calculate progression to each next state.

Interestingly enough, though this problem consisted of many calculations, we saw little room for OpenCL. This came from the fact that there were many memory calls and branches (if statements, `std::max`, etc). TBB, with a hard-coded chunk size of 256, was used to iterate over both of the for loops. We found that the `option_explicit` algorithm was the only one that required a chunk size over 128. This is probably due to the fact that the `std::pow` calculation used at the start of the second for loop is quite heavy.

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/final/option_explicit.png)

### Puzzle: Circuit Sim
- TBB used to parallelise calculation of next_state.

Due to the complex nature of the way flip flops and nand gates are connected in this algorithm, we struggled to optimise the inherent algorithm. However, two approaches were taken to optimise this algo. Firstly,  `tbb::taskgroup` was implemented to calculate the source of each nand input. This actually gave **worse** results than the reference algorithm. In trying to determine a way to stop `taskgroup` from splitting tasks unneccessarily (perhaps after a certain depth), we decided to move away from taskgroup for other approaches. It was impossible to tell circuit depth (i.e. nodes until a flip flop) with only information on nand gate index, as these are set up in a completely random way.

The second approach taken was to parallelise the calculation of flip-flip inputs in the `next` function. This means that each value of the `next_state` variable is calculated in parallel (input to flip flop one is not affected by input to flip flop two). There were no (*obvious*) memory optimisations to be implemented here.

It's important to note that `std::vector<bool>` is not handled well. As such, a change in the underlying values (from bool to int) was made. This was required, as otherwise the output would sometime be incorrect (see notes in `life` for more information on errors with std::vector<bool>).

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/final/circuit_sim.png)

### Puzzle: Median Bits

- TBB to parallelise creation of the vector using the seet

Again, no obious algorithm improvement was seen here. Perhaps with some further time and work we could have looked at a better algorithm to calculate temp without as many loops (the XOR operations in side the inner loop seem to carry on no dependancy and so can't be minimised). 

We used `TBB::parallel_for` to improve the speed of the creation of the `tmp` vector. As sorting is dependant on relative values, this couldn't be further optimised. 

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/final/median_bits.png)

# Testing

Testing formed an important part of the project: optimised code which leads to incorrect output would cause an upset Head of Sales. The knowledge that VBA is more correct than OpenCL is enough to make any grown adult weep.

In order to save time regenerating reference outputs each time (as is the case with `bin/run_puzzle`), a set of input data and reference output data was created. The binary objects were too large to commit to the repository, but sit on [txl11's personal site on DoC](http://www.doc.ic.ac.uk/~txl11/) and are downloaded (handily via the `get_ref_data.sh` script), uncompressed on each machine being used for testing. The script `generate_ref_input_output.sh` runs `bin/create_puzzle_input` and `bin/execute_puzzle` for various sizes as specified for each puzzle (usually based loosely on reference execution time).

More scripts then use this test data as its input and reference. `test_tbb_chunks.sh` varies the environment variables to change the TBB chunk size, as well as enable and disable OpenCL, and `test_output.sh` simply runs the non_reference version of code (this being used at the end once all chunksize parameters etc have been hardcoded in). The output of the non_reference execution is then sent to `bin/compare_puzzle` where it is compared to the reference output binary generated earlier. The output logs of all stages of code execution are saved and committed (usually in a tar.gz file to save space) for further local analysis.

Local analysis takes the form of Python scripts which parse the log files to check that the code executed correctly (ie our output matches the reference output), and then parses the logs to work out the reference execution time, and our code execution time. If the output does not match a given input, this is raised via the terminal and that particular data point is not plotted. This has proven particular useful at identifying edge cases which otherwise may not have been spotted.

The Python scripts `plot_chunks.py` and `plot_final_results.py` are used for parsing the majority of data - both for plotting differing chunk values and the final code against reference execution time.


# Appendix: Optimisation Conclusions

The following graphs show some of the results from the testing we ran on our code. The final results are slightly different, as our final code did not have the overhead required to allow for granularity. Nonetheless, the following graphs for each algorithm help explain our conclusions when it comes to TBB chunk size, as well as swapping over to Open CL.

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/chunked/life.png)

| Algo Name | TBB Chunk Size | OpenCL |
| --------- |----------------|--------|
|life       | 128            | after n=250 |


![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/chunked/matrix_exponent.png)

| Algo Name | TBB Chunk Size | OpenCL |
| --------- |----------------|--------|
|matrix_exponent| N/A        | after n=500   |


![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/chunked/string_search.png)

| Algo Name | TBB Chunk Size | OpenCL |
| --------- |----------------|--------|
|string_search| 16 (only when 900<n<1,200,000)           | N/A    |

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/chunked/option_explicit.png)

| Algo Name | TBB Chunk Size | OpenCL |
| --------- |----------------|--------|
|option_explicit| 512            | N/A    |

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/chunked/circuit_sim.png)

| Algo Name | TBB Chunk Size | OpenCL |
| --------- |----------------|--------|
|circuit_sim| 128            | N/A    |

![](http://www.doc.ic.ac.uk/~txl11/hpce_6_imgs/chunked/median_bits.png)

| Algo Name | TBB Chunk Size | OpenCL |
| --------- |----------------|--------|
|median_bits| 128            | N/A    |
