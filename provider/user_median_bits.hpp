#ifndef user_median_bits_hpp
#define user_median_bits_hpp
#include "puzzler/puzzles/median_bits.hpp"


class MedianBitsProvider
  : public puzzler::MedianBitsPuzzle
{
public:
  MedianBitsProvider()
  {}

  virtual void Execute(
    puzzler::ILog *pLog,
    const puzzler::MedianBitsInput *input,
    puzzler::MedianBitsOutput *output
  )
  const override {
    /* ***********************************************************************
     * Values for TBB and Openl CL have been hardcoded to work best on AWS.
     * 
     * See readme.md for explanation, and the following URL to see what our
     * test code looked like
     *
     * https://github.com/HPCE/hpce_2014_cw6_dm1911_txl11/commit/4307ba0ce965ff887d8055a0118fc2b0c3ded31d
     * *********************************************************************** */
    
    pLog->LogInfo("Generating bits.");
    unsigned numberOfXorIterations = (unsigned)(log(16 + input->n) / log(1.1));
    double tic = puzzler::now();
    std::vector<uint32_t> temp(input->n);
    typedef tbb::blocked_range<unsigned> my_range_t;

    if (input->n < 4) {
      // strangeness happens when running tbb with small values. let's fallback.
      ReferenceExecute(pLog, input, output); return;
    }

    temp[0] = 0;
    auto f = [&](const my_range_t &chunk) {
      uint32_t t, x, y, z, w;

      for (unsigned i = chunk.begin(); i < chunk.end(); i++) {
        x = i * (7 + input->seed);
        y = 0;
        z = 0;
        w = 0;
        t = 0;

        for (unsigned j = 0; j < numberOfXorIterations; j++) {
          t = x ^ (x << 11); // XOR. This is quite fast I feel.
          x = y; y = z; z = w;
          w = w ^ (w >> 19) ^ t ^ (t >> 8);
        }

        temp[i] = w;
      }
    };

    my_range_t range(1, input->n, 128); 
    tbb::parallel_for(range, f, tbb::simple_partitioner());

    pLog->LogInfo("Finding median, delta=%lg", puzzler::now() - tic);
    tic = puzzler::now();

    std::sort(temp.begin(), temp.end()); // this is Nlog(N) - I doubt this can be improved.

    output->median = temp[temp.size() / 2];
    pLog->LogInfo("Done, median=%u (%lg), delta=%lg", output->median, output->median / pow(2.0, 32), puzzler::now() - tic);
  }


};

#endif
