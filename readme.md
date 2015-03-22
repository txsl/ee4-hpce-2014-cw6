# HPCE6 
After woking diligently on optimising the given algorithms, we returned our code to the sales and marketing team. As these algorithms are highly commercially sensitive, details of our conversations must remain secret. However, vocabulary used in the meeting room upon presenting our results included "bloody amazing" and "best thing since Jesus". We've seen a 100% market takeover in only 48 hours, and the optimisations have been critical in raising a valutation of £1.2tn. The sales team were more than happy, now earning a healthy commission on sales of these excellent programms. 
Furthermore, the CEO of the company has personally applauded us as volontary students, and has offered us full time positions as executives. Honoured by the offer, but bound by Peter Cheung to decline, we instead accepted a program licence to use the below 6 algorithms without limit. Considering this would otherwise cost a full £15.00, we think the project has been a good experience for us.
We move forward now looking at applying our excellent optimisation strategies at other prestigious software companies such as Kodak or HP.
# Approach 
After understanding each agorithm's functionality, we searched for :
- Potential Algorithm Improvements
- Potential TBB use
- Potential OpenCL use

A number of attempts were made to improve the speed of algorithms (see testing), and **optimised results were hard coded to provide the best possible speed on AWS g2.2xlarge instances** (*this is a competition, after all!*). Each optimised algorithm has code that allows the fine tuning of TBB and OpenCL parameters, and these were used to determine optimum values. In an attempt to save set up time though, these have been commented out for our final submission. Sales people don't know how to use `export` anyhow, so all is good.
## The Puzzles
Description of each optimisation explored and what was then actually done. Final plot of input vs output and speedup as well.
### Life
`life` is a classic stencil type operation, with each cell's next state in the grid needing to be computed for each time step. There is a clear loop dependency between each time step, but the calculation of each cell within a given time step can be calculated in parallel. Two parallelisation methods were tested: Threaded Building Blocks Parallel For loop, and OpenCL.

In order for either optimisation to be applied, a modification had to be made. Since `life` looks at whether an individual cell is 'alive' or not, it was originally stored in a Vector of Bools. It is [well known](https://www.google.co.uk/?q=c%2B%2B+vector+bool) that the C++ implementation of a Vector of Bools is [not recommended](http://www.codingstandard.com/rule/17-1-1-do-not-use-stdvector/), and does not guarantee safe modification of values concurrently. Thus it must be converted to another datatype. In this case, they wer converted to ints. Modification of the `update` function was also necessary for this to work (although some implicit type conversion on its return value does take place).

The TBB implementation was created using `tbb::parallel_for`, with the outer loop being parallel-ised. In other words, a chunk represents a certain number of columns, and the inner for loop executes across those rows (and these are running in chunked parallel groups).

An OpenCL implementation would potentially run even faster, since it can operate across both dimensions in one operation, since it has plenty of cores. It also works well since we feed in the starting conditions (data about each point), and save the computed next state. By using pointers carefully, we can swap the pointers to each vector of data between iterations, and avoid the need to read the data to and from the GPU until the very end. Only then do we read the data from the GPU's memory back to be saved to the output.

Based on tests run on AWS, we determined that for problem sizes of less than 250, the TBB `parallel_for` method will be used, and for problems larger the OpenCL implementation will be used. OpenCL takes longer to initialise, but for large problem sets pays off with significant gains in execution time.

For problem sizes where TBB is used, the optimum value of 

(graph)
### Matrix Exponent
- Algorithm Optimisation (O(N^3) to O(N^2) for matrix multiplication)
- OpenCL with some clever modifications.

Matrix Exponent was an interesting programm to look at. Interestingly, the David Thomas Matrix Multiplication method was used to calculate the exponent of each matrix. Though many mathematicians would be appalled by the use of such inprecise methods, the computer scientists in us were delighted to see the number of optimisations that could be done here. Specicially, as each column in the accumulator matrix was similar, the N \* N \* N operation was reduced to N \* N. We then looked at converting this to OpenCL, as this problem consits of many smaller calculations without the need to move much data around. There was little room for parallelisation between each loop (as each loop of the matrix exponent was dependant on the accumulator matrix), so TBB was not considered.
In using OpenCL, buffers are swapped and only the first element is read from the matrix. This is as the returning hash only takes the first element of the accumulator matrix at each iteration.

    for (unsigned i = 2; i < input->steps; i++) {
      kernel.setArg(0, buffCurrVector);
      kernel.setArg(2, buffNextVector);
      queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize); 
      queue.enqueueBarrier();
      //read only the first element straight into hash[i].
      queue.enqueueReadBuffer(buffNextVector, CL_TRUE, 0, sizeof(uint32_t), &hash[i]);
      std::swap(buffCurrVector, buffNextVector);
    }
    
(graph)
### String Search

String Search is searching for specific DNA patterns in a long string which represents a DNA sequence. For a given string, there is more than one pattern to find. The way in which the search code works is quite specific:

It searches for a pattern match, iterating through each pattern at a given index in the string, looking for to match a pattern. It does this one by one (ie at a given point in the string, it iterates through each pattern looking for a match). As soon as it finds a match, it stops looking for patterns at that index, and updates the index (of where it is located in the string) to the next location after the matched pattern. If no pattern is found, it increments the index by one (ie moves along by one) and starts looking for a pattern once again.

If we are lucky and the pattern match occurs for the first pattern in our series, we will avoid unnecessary calculations. On the other end of the spectrum, if the last pattern is the one matched each time, all of the other pattern searches (for different patterns) were futile, and most importantly for us, a waste of time.

The nature of operation (searching through each pattern _until_ a match is found) makes it difficult to run in parallel, since knowing when to stop searching may not occur in the correct order.

The first strategy attempted was to calculate the match of each pattern in each string position 

### Option Explicit
- TBB to calculate initial state
- TBB to calculate future states (inner loop)
In the algorithm used to determin the value of derivatives of the company, a fast execution time was vital. Interestingly enough, though this problem consisted of many calculations, we saw little room for OpenCL. This came from the fact that there were many memory calls and branches (if statements, `std::max`, etc). We turned to TBB to parallelise two loops; the first one that sets up `state`, and the inner `for` loop in the calculation of future states. 
### Circuit Sim
- TBB used to parallelise calculation of next_state.
Due to the complex nature of the way flip flops and nand gates are connected in this algorithm, we struggled to find a better algorithm here. However, we took two approaches to optimised. Firstly, we looked at using `tbb::taskgroup` to calculate the source of each nand input. This actually gave **worse** results than the reference algorithm. In trying to determine a way to stop `taskgroup` from splitting tasks unneccessarily (perhaps after a certain depth), we decided to move away from taskgroup for other approaches. It was impossible to tell circuit depth (i.e. nodes until a flip flop) with only information on nand gate index.
The second approach taken was to parallelise the calculation of flip-flip inputs in the `next` function. This means that the calculation of the `next_state` variable is calculated in parallel (input to flip flop one is not affected by input to flip flop two). There were no (*obvious*) memory optimisations here.
It's important to note that TBB does not handle the `bool` type very well. As such, a change in the underlying values (from bool to int) was made. This was required, as otherwise the output would sometime be incorrect.
(graph)
### Median Bits
- TBB to parallelise creation of the vector using the seet
Again, no obious algorithm improvement was seen here. Perhaps with some further time and work we could have looked at a better algorithm to calculate temp without as many loops (the XOR operations in side the inner loop seem to carry on no dependancy and so can't be minimised). 
We used `TBB::parallel_for` to improve the speed of the creation of the `tmp` vector. As sorting is dependant on relative values, this couldn't be further optimised. 
## Testing
What we tested and how we tested it