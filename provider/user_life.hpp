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

    unsigned n=input->n;
    std::vector<bool> state=input->state;

    log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
      dst<<"\n";
      for(unsigned y=0; y<n; y++){
        for(unsigned x=0; x<n; x++){
          dst<<(state.at(y*n+x)?'x':' ');
        }
        dst<<"\n";
      }
    });

    std::vector<int> int_state(state.begin(), state.end());

    for(unsigned i=0; i<input->steps; i++){
      log->LogVerbose("Starting iteration %d of %d\n", i, input->steps);

      std::vector<int> next(n*n);

      std::vector<bool> orig_next(n*n);

      auto impl = [&](int x){
        for(unsigned y=0; y<n; y++){
          next[y*n+x]=int_update(n, int_state, x, y);
        }
      };

      tbb::parallel_for <size_t> (0, n, impl);

      for(unsigned x=0; x<n; x++){
        for(unsigned y=0; y<n; y++){
          orig_next[y*n+x]=update(n, state, x, y);
        }
      }

      state = orig_next;
      int_state = next;

      // The weird form of log is so that there is little overhead
      // if logging is disabled
      log->Log(puzzler::Log_Info, [&](std::ostream &dst){
        dst<<"\n";
        for(unsigned y=0; y<n; y++){
          for(unsigned x=0; x<n; x++){
            dst<<(next[y*n+x]?'x':' ');
            // dst<<(orig_next[y*n+x]?'o_x':' ');
          }
          dst<<"\n";
        }
      });

      log->Log(puzzler::Log_Info, [&](std::ostream &dst){
        dst<<"\n";
        for(unsigned y=0; y<n; y++){
          for(unsigned x=0; x<n; x++){
            // dst<<(next[y*n+x]?'x':' ');
            dst<<(orig_next[y*n+x]?'o':' ');
          }
          dst<<"\n";
        }
      });

      log->Log(puzzler::Log_Info, [&](std::ostream &dst){
        dst<<"\n";
        for(unsigned y=0; y<n; y++){
          for(unsigned x=0; x<n; x++){
            dst<<(orig_next[y*n+x] == next[y*n+x]?' ':'x');
          }
          dst<<"\n";
        }
      });
    
      // log->Log(puzzler::Log_Info, [&](std::ostream &dst){
      //   bool b_n = (next != 0);
      //   if(orig_next == b_n){
      //     dst<<"equal";
      //   } else {
      //     dst<<"not equal";
      //   }
      //   dst<< "Orig size: " << orig_next.size() << " New size: " << next.size();
      // });

    }

    log->LogVerbose("Finished steps");
    log->LogVerbose("This is a custom life");

    // state(int_state.begin(), int_state.end());

    for(unsigned x=0; x<n; x++){
      for(unsigned y=0; y<n; y++){
        state[y*n+x]=int(int_state[y*n+x]);
      }
    }    

    output->state=state;
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
