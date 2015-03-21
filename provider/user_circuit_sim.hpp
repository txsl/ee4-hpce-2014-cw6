#ifndef user_circuit_sim_hpp
#define user_circuit_sim_hpp

#include "puzzler/puzzles/circuit_sim.hpp"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

class CircuitSimProvider
  : public puzzler::CircuitSimPuzzle
{
protected:

  int calcSrc(unsigned src, const std::vector<int> &state, const puzzler::CircuitSimInput *input) const
  {
    if (src < input->flipFlopCount) {
      return state.at(src);
    } else {
      unsigned nandSrc = src - input->flipFlopCount;
      int a = 1 & calcSrc(input->nandGateInputs.at(nandSrc).first, state, input);
      int b = 1 & calcSrc(input->nandGateInputs.at(nandSrc).second, state, input);
      return !(a && b);
    }
  }


  std::vector<int> tbb_next(int chunkSize, const std::vector<int> &state, const puzzler::CircuitSimInput *input) const
  {
    std::vector<int> res(state.size());

    typedef tbb::blocked_range<unsigned> my_range_t;
    my_range_t range(0,res.size(),chunkSize);

    auto f=[&](const my_range_t &chunk){
      for (unsigned i = chunk.begin(); i < chunk.end(); i++) {
        res[i] = calcSrc(input->flipFlopInputs[i], state, input);
      }
    };
    tbb::parallel_for(range, f, tbb::simple_partitioner());

    return res;
  }

public:
  CircuitSimProvider()
  {}

  virtual void Execute(
    puzzler::ILog *log, 
    const puzzler::CircuitSimInput *input, 
    puzzler::CircuitSimOutput *output
  ) const override {
    char *v = getenv("HPCE_CHUNKSIZE_K");
    int chunkSize = 16;
    if(v==NULL) {
      log->LogInfo("No HPCE_CHUNKSIZE_K envrionment variable found. Default to %i", chunkSize);
    } else {
      chunkSize = atoi(v);
      log->LogInfo("HPCE_CHUNKSIZE_K environment variable found and set to %i ", chunkSize);
    }



    log->LogVerbose("About to start running clock cycles (total = %d)", input->clockCycles);

    std::vector<int> state(input->inputState.begin(), input->inputState.end());

    for (unsigned i = 0; i < input->clockCycles ; i++) {
      log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

      state = tbb_next(chunkSize, state, input);

      log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
        for (unsigned i = 0; i < state.size(); i++) {
          dst << state[i];
        }
      });
    }

    log->LogVerbose("Finished clock cycles");

    std::vector<bool> state_tmp(state.begin(), state.end());
    output->outputState = state_tmp;
  }

};

#endif
