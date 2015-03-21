#ifndef user_circuit_sim_hpp
#define user_circuit_sim_hpp

#include <math.h>       /* ceil */

#include "puzzler/puzzles/circuit_sim.hpp"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

class CircuitSimProvider
  : public puzzler::CircuitSimPuzzle
{
protected:
  std::vector<bool> tbb_next(const std::vector<bool> &state, const puzzler::CircuitSimInput *input) const
  {
    std::vector<bool> res(state.size());

    typedef tbb::blocked_range<unsigned> my_range_t;
    my_range_t range(0,res.size(),512);

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
    // for (unsigned i = 0; i < input->flipFlopInputs.size(); i++) {
    //   log->LogVerbose("Input Flip Flop, %i, %i", (int)input->flipFlopInputs[i], input->flipFlopCount);
    // }
    log->LogVerbose("About to start running clock cycles (total = %d)", input->clockCycles);

    std::vector<bool> state = input->inputState;


    for (unsigned i = 0; i < input->clockCycles ; i++) {
      log->LogVerbose("Starting iteration %d of %d\n", i, input->clockCycles);

      state = tbb_next(state, input);

      log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
        for (unsigned i = 0; i < state.size(); i++) {
          dst << state[i];
        }
      });
    }

    log->LogVerbose("Finished clock cycles");

    output->outputState = state;
  }

};

#endif
