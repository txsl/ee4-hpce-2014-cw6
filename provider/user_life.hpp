#ifndef user_life_hpp
#define user_life_hpp

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

    for(unsigned i=0; i<input->steps; i++){
      log->LogVerbose("Starting iteration %d of %d\n", i, input->steps);

      std::vector<bool> next(n*n);

      for(unsigned x=0; x<n; x++){
        for(unsigned y=0; y<n; y++){
          next[y*n+x]=update(n, state, x, y);
        }
      }

      state=next;

      // The weird form of log is so that there is little overhead
      // if logging is disabled
      log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
        dst<<"\n";
        for(unsigned y=0; y<n; y++){
          for(unsigned x=0; x<n; x++){
            dst<<(state[y*n+x]?'x':' ');
          }
          dst<<"\n";
        }
      });
    }

    log->LogVerbose("Finished steps");
    log->LogVerbose("This is a custom life");

    output->state=state;
  }

};

#endif
