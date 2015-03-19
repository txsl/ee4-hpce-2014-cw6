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
    // ReferenceExecute(log, input, output);
    int n = pInput->n;
    double u = pInput->u, d = pInput->d;

    log->LogInfo("Params: pInput->u=%lg, u=%lg, d=%lg, wU=%lg, wM=%lg, wD=%lg", pInput->u, u, pInput->d, pInput->wU, pInput->wM, pInput->wD);

    std::vector<double> state(n * 2 + 1);
    double vU = pInput->S0, vD = pInput->S0;
    state[pInput->n] = std::max(vU - pInput->K, 0.0);
    
    int K;
    char *v;

    v=getenv("HPCE_CHUNKSIZE_K");
    if(v==NULL){
      K = 16;
      log->LogInfo("No HPCE_CHUNKSIZE_K envrionment variable found");
        // printf("HPCE_FFT_LOOP_K not set. Using a size of %i instead.\n", chunk_size);
    }else{
      K = atoi(v);
      log->LogInfo("HPCE_CHUNKSIZE_K environment variable found");
        // printf("Using a chunk size of %i (set in the environment variable 'HPCE_FFT_LOOP_K'.\n)", chunk_size);
    }
    log->LogInfo("Chunksize set as K=%i", K);

    typedef tbb::blocked_range<unsigned> my_range_t;
    my_range_t range(1, n+1, K);

    auto impl = [&](const my_range_t &chunk)
    {
      // log->LogInfo("begin=%i, end=%i", chunk.begin(), chunk.end());

      double local_vU = vU * pow(u, chunk.begin() - 1);
      double local_vD = vD * pow(d, chunk.begin() - 1);

      for (int i = chunk.begin(); i < chunk.end(); i++) {

        local_vU = local_vU * u;
        local_vD = local_vD * d;

        state[n + i] = std::max(local_vU - pInput->K, 0.0);
        state[n - i] = std::max(local_vD - pInput->K, 0.0);
      }
    };

    tbb::parallel_for(range, impl, tbb::simple_partitioner());

    // double local_vU = vU;
    // double local_vD = vD;
    // double fixed_vU = vU;
    // double fixed_vD = vD;

    // log->LogInfo("Params: u=%lg", u);

    // for (int i = 1; i <= n; i++) {
    //   log->LogInfo("u=%lg, u^=%lg, d=%lg, d^=%lg", u, pow(u, i), d, pow(d, i));
    //   log->LogInfo("%f", pow(10.0, 2.0));

    //   local_vU = fixed_vU * pow(u, i);
    //   local_vD = fixed_vD * pow(d, i);
      
    //   vU = vU * u;
    //   vD = vD * d;

    //   log->LogInfo("vU=%lg, local_vU=%lg, vD=%lg, local_vD=%lg", vU, local_vU, vD, local_vD);

    //   state[n + i] = std::max(local_vU - pInput->K, 0.0);
    //   state[n - i] = std::max(local_vD - pInput->K, 0.0);
    // }

    typedef tbb::blocked_range<unsigned> my_range_t;
    my_range_t inner_range(0, n, K);

    double wU = pInput->wU, wD = pInput->wD, wM = pInput->wM;

    for (int t = n - 1; t >= 0; t--) {
      std::vector<double> tmp = state;

      // vU = pInput->S0, vD = pInput->S0;

      auto inner_impl = [&](const my_range_t &chunk)
      {

        double local_vU = vU * pow(u, chunk.begin());
        double local_vD = vD * pow(d, chunk.begin());

        for(int i=chunk.begin(); i < chunk.end(); i++)
        {

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

      tbb::parallel_for(inner_range, inner_impl, tbb::simple_partitioner());
      
      // double local_vU, local_vD;

      // for (int i = 0; i < n; i++) {

      //   local_vU = vU * pow(u, i);
      //   local_vD = vD * pow(d, i);

      //   double vCU = wU * state[n + i + 1] + wM * state[n + i] + wD * state[n + i - 1];
      //   double vCD = wU * state[n - i + 1] + wM * state[n - i] + wD * state[n - i - 1];
      //   vCU = std::max(vCU, local_vU - pInput->K);
      //   vCD = std::max(vCD, local_vD - pInput->K);
      //   tmp[n + i] = vCU;
      //   tmp[n - i] = vCD;

      //   // vU = vU * u;
      //   // vD = vD * d;
      // }

      state = tmp;
    }

    pOutput->steps = n;
    pOutput->value = state[n];

    log->LogVerbose("Priced n=%d, S0=%lg, K=%lg, r=%lg, sigma=%lg, BU=%lg : value=%lg", n, pInput->S0, pInput->K, pInput->r, pInput->sigma, pInput->BU, pOutput->value);

  }

};

#endif
