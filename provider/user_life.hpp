#ifndef user_life_hpp
#define user_life_hpp

#include "tbb/parallel_for.h"

#include "puzzler/puzzles/life.hpp"

class LifeProvider
  : public puzzler::LifePuzzle
{
public:
  LifeProvider()
  {}

  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::LifeInput *input,
		       puzzler::LifeOutput *output
		       ) const override {
    log->LogVerbose("About to start running iterations (total = %d)", input->steps);

    int K;

    char *v=getenv("HPCE_LIFE_CHUNKSIZE_K");
    if(v==NULL){
      K = 16;
      log->LogInfo("No HPCE_LIFE_CHUNKSIZE_K envrionment variable found");
        // printf("HPCE_FFT_LOOP_K not set. Using a size of %i instead.\n", chunk_size);
    }else{
      K = atoi(v);
      log->LogInfo("HPCE_LIFE_CHUNKSIZE_K environment variable found");
        // printf("Using a chunk size of %i (set in the environment variable 'HPCE_FFT_LOOP_K'.\n)", chunk_size);
    }
    log->LogInfo("Chunksize set as K=%i", K);
    unsigned n=input->n;
    // std::vector<bool> state=input->state;
    std::vector<int> state(input->state.begin(), input->state.end());

    log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
      dst<<"\n";
      for(unsigned y=0; y<n; y++){
        for(unsigned x=0; x<n; x++){
          dst<<(state.at(y*n+x)?'x':' ');
        }
        dst<<"\n";
      }
    });

    for(unsigned i=0; i<input->steps; i++){
      log->LogVerbose("Starting iteration %d of %d\n", i, input->steps);

      std::vector<int> next(n*n);

      typedef tbb::blocked_range<unsigned> my_range_t;
      my_range_t range(0, n, K);

      auto impl = [&](const my_range_t &chunk){
        for(unsigned x=chunk.begin(); x!=chunk.end(); x++ ){
          for(unsigned y=0; y<n; y++){
            next[y*n+x]=int_update(n, state, x, y);
          }
        }
      };

      // tbb::parallel_for <size_t> (0, n, impl);
      tbb::parallel_for(range, impl, tbb::simple_partitioner());
      
      state = next;

      // The weird form of log is so that there is little overhead
      // if logging is disabled
      log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
        dst<<"\n";
        for(unsigned y=0; y<n; y++){
          for(unsigned x=0; x<n; x++){
            dst<<(next[y*n+x]?'x':' ');
            // dst<<(orig_next[y*n+x]?'o_x':' ');
          }
          dst<<"\n";
        }
      });

    }

    log->LogVerbose("Finished steps");
    log->LogVerbose("This is a custom life");

    std::vector<bool> state_tmp(state.begin(), state.end());

    output->state = state_tmp;
  }

  int int_update(int n, const std::vector<int> &curr, int x, int y) const
  {
    int neighbours=0;
    for(int dx=-1;dx<=+1;dx++){
      for(int dy=-1;dy<=+1;dy++){
        int ox=(n+x+dx)%n; // handle wrap-around
        int oy=(n+y+dy)%n;

        if(curr.at(oy*n+ox) && !(dx==0 && dy==0))
          neighbours++;
      }
    }

    if(curr[n*y+x]){
      // alive
      if(neighbours<2){
        return false;
      }else if(neighbours>3){
        return false;
      }else{
        return true;
      }
    }else{
      // dead
      if(neighbours==3){
        return true;
      }else{
        return false;
      }
    }
  }

};

#endif
