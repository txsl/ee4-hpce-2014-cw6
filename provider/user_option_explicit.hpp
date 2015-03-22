#ifndef user_option_explicit_hpp
#define user_option_explicit_hpp

#include <random>
#include <math.h>

#include "puzzler/core/puzzle.hpp"
#include "puzzler/puzzles/option_explicit.hpp"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

class OptionExplicitProvider
  : public puzzler::OptionExplicitPuzzle
{
public:
  virtual void Execute(
    puzzler::ILog *log,
    const puzzler::OptionExplicitInput *pInput,
    puzzler::OptionExplicitOutput *pOutput
  ) const override {
    /* ***********************************************************************
     * Values for TBB and Openl CL have been hardcoded to work best on AWS.
     * 
     * See readme.md for explanation, and the following URL to see what our
     * test code looked like
     *
     * https://github.com/HPCE/hpce_2014_cw6_dm1911_txl11/commit/4307ba0ce965ff887d8055a0118fc2b0c3ded31d
     * *********************************************************************** */
    
    typedef tbb::blocked_range<int> my_range_t;
    int n     = pInput->n;
    double u  = pInput->u, d = pInput->d;
    double vU = pInput->S0, vD = pInput->S0;

    std::vector<double> state(n * 2 + 1);
    state[pInput->n] = std::max(vU - pInput->K, 0.0);

    log->LogInfo("Params: pInput->u=%lg, u=%lg, d=%lg, wU=%lg, wM=%lg, wD=%lg", pInput->u, u, pInput->d, pInput->wU, pInput->wM, pInput->wD);

    auto impl = [&](const my_range_t &chunk) {
      double local_vU = vU * pow(u, chunk.begin() - 1);
      double local_vD = vD * pow(d, chunk.begin() - 1);

      for (int i = chunk.begin(); i < chunk.end(); i++) {
        local_vU = local_vU * u;
        local_vD = local_vD * d;
        state[n + i] = std::max(local_vU - pInput->K, 0.0);
        state[n - i] = std::max(local_vD - pInput->K, 0.0);
      }
    };

    my_range_t range(1, n + 1, 256);
    tbb::parallel_for(range, impl, tbb::simple_partitioner());


    /* ************************************
             NOW DOING INNER LOOP
       ************************************ */

    double wU = pInput->wU, wD = pInput->wD, wM = pInput->wM;
    std::vector<double> tmp = state;

    auto inner_impl = [&](const my_range_t &chunk) {
      double local_vU = vU * pow(u, chunk.begin());
      double local_vD = vD * pow(d, chunk.begin());
    
      for (int i = chunk.begin(); i < chunk.end(); i++) {
        double vCU = wU * state[n + i + 1] + wM * state[n + i] + wD * state[n + i - 1];
        double vCD = wU * state[n - i + 1] + wM * state[n - i] + wD * state[n - i - 1];
        vCU = std::max(vCU, local_vU - pInput->K);
        vCD = std::max(vCD, local_vD - pInput->K);
        tmp[n + i] = vCU;
        tmp[n - i] = vCD;

        local_vU = local_vU * u;
        local_vD = local_vD * d;
      }
    };

    my_range_t inner_range(0, n, 256);
    for (int t = n - 1; t >= 0; t--) {
      tbb::parallel_for(inner_range, inner_impl, tbb::simple_partitioner());
      std::swap(state, tmp);
    }

    pOutput->steps = n;
    pOutput->value = state[n];

    log->LogVerbose("Priced n=%d, S0=%lg, K=%lg, r=%lg, sigma=%lg, BU=%lg : value=%lg", n, pInput->S0, pInput->K, pInput->r, pInput->sigma, pInput->BU, pOutput->value);
  }

};

#endif
