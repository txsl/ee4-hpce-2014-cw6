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
    pLog->LogInfo("Generating bits.");
    double tic = puzzler::now();

    std::vector<uint32_t> temp(input->n);

    // for (unsigned n = 0; n < 100; n++) {
    //   pLog->LogInfo("Result for n=%i is %i", n, (unsigned)(log(16 + n) / log(1.1)));
    // }

    char *v;
    int K;

    v=getenv("HPCE_CHUNKSIZE_K");

    if(v==NULL){
      K = 16;
      pLog->LogInfo("No HPCE_CHUNKSIZE_K envrionment variable found");
        // printf("HPCE_FFT_LOOP_K not set. Using a size of %i instead.\n", chunk_size);
    }else{
      K = atoi(v);
      pLog->LogInfo("HPCE_CHUNKSIZE_K environment variable found");
        // printf("Using a chunk size of %i (set in the environment variable 'HPCE_FFT_LOOP_K'.\n)", chunk_size);
    }
    pLog->LogInfo("Chunksize set as K=%i", K);


    typedef tbb::blocked_range<unsigned> my_range_t;
    my_range_t range(0,input->n,ceil(input->n/4));

    auto f=[&](const my_range_t &chunk){
      uint32_t x,y,z,w;

      for (unsigned i = chunk.begin(); i < chunk.end(); i++) {
        x = i * (7 + input->seed);
        y = 0;
        z = 0;
        w = 0;

        for (unsigned j = 0; j < (unsigned)(log(16 + input->n) / log(1.1)); j++) {
          uint32_t t = x ^ (x << 11);
          x = y; y = z; z = w;
          w = w ^ (w >> 19) ^ t ^ (t >> 8);
        }

        temp[i] = w;
      }
    };
    tbb::parallel_for(range, f, tbb::simple_partitioner()); 

    pLog->LogInfo("Finding median, delta=%lg", puzzler::now() - tic);
    tic = puzzler::now();


    // this is Nlog(N) - I doubt this can be improved.
    std::sort(temp.begin(), temp.end());

    output->median = temp[temp.size() / 2];

    pLog->LogInfo("Done, median=%u (%lg), delta=%lg", output->median, output->median / pow(2.0, 32), puzzler::now() - tic);
  }


};

#endif
