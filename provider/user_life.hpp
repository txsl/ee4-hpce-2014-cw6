#ifndef user_life_hpp
#define user_life_hpp

#include "tbb/parallel_for.h"

#include "opencl_frame.hpp"

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

    char *v;
    int OPT;
    unsigned n = input->n;
    // std::vector<bool> state=input->state;
    std::vector<int> state(input->state.begin(), input->state.end());
    std::vector<int> next(n * n);

    // To determine if we are usign TBB or OpenCL
    v = getenv("HPCE_CL_ENABLE");

    if(v==NULL)
    {
      log->LogInfo("No HPCE_CL_ENABLE detected");
      OPT = 0;
    }
    else
    {
      log->LogInfo("HPCE_CL_ENABLE detected");
      OPT = atoi(v);
    }

    if (!OPT) {
      log->LogInfo("Using TBB implementation");

      int K = 16;
      v = getenv("HPCE_CHUNKSIZE_K");
      if (v == NULL) {
        log->LogInfo("No HPCE_CHUNKSIZE_K envrionment variable found");
      } else {
        K = atoi(v);
        log->LogInfo("HPCE_CHUNKSIZE_K environment variable found as %i", K);
      }

      log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
        dst << "\n";
        for (unsigned y = 0; y < n; y++) {
          for (unsigned x = 0; x < n; x++) {
            dst << (state.at(y * n + x) ? 'x' : ' ');
          }
          dst << "\n";
        }
      });

      for (unsigned i = 0; i < input->steps; i++) {
        log->LogVerbose("Starting iteration %d of %d\n", i, input->steps);

        typedef tbb::blocked_range<unsigned> my_range_t;
        my_range_t range(0, n, K);

        auto impl = [&](const my_range_t &chunk) {
          for (unsigned x = chunk.begin(); x != chunk.end(); x++ ) {
            for (unsigned y = 0; y < n; y++) {
              next[y * n + x] = int_update(n, state, x, y);
            }
          }
        };

        tbb::parallel_for(range, impl, tbb::simple_partitioner());

        state = next;
      }
    }
    else
    {
      log->LogInfo("Using OpenCL implementation");

      size_t cbBuffer = n * n * sizeof(int);
      puzzler::OpenclHelper opencl(log);
      opencl.selectPlatform().selectDevice().makeContext();
      opencl.build("life_kernel.cl");

      cl::CommandQueue queue(opencl.getContext(), opencl.getDevice());
      cl::Buffer buffCurr(opencl.getContext(), CL_MEM_READ_WRITE, cbBuffer);
      cl::Buffer buffNext(opencl.getContext(), CL_MEM_READ_WRITE, cbBuffer);
      queue.enqueueWriteBuffer(buffCurr, CL_FALSE, 0, cbBuffer, &state[0], NULL);
      
      cl::Kernel kernel(opencl.getProgram(), "life_update");
      kernel.setArg(0, n);
      cl::NDRange offset(0, 0);
      cl::NDRange globalSize(n, n);
      cl::NDRange localSize = cl::NullRange;

      for (unsigned t = 0; t < n; t++) {
        log->LogVerbose("On iteration %i. n=%i", t, n);

        kernel.setArg(1, buffCurr);
        kernel.setArg(2, buffNext);

        queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize); //, &kernelDependencies, &evExecutedKernel);
        queue.enqueueBarrier();

        std::swap(buffCurr, buffNext);
      }

      queue.enqueueReadBuffer(buffCurr, CL_TRUE, 0, cbBuffer, &state[0]);
    }


    log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
      dst << "\n";
      for (unsigned y = 0; y < n; y++) {
        for (unsigned x = 0; x < n; x++) {
          dst << (next[y * n + x] ? 'x' : ' ');
        }
        dst << "\n";
      }
    });

    log->LogVerbose("Finished steps");

    std::vector<bool> state_tmp(state.begin(), state.end());

    output->state = state_tmp;
  }


  int int_update(int n, const std::vector<int> &curr, int x, int y) const
  {
    int neighbours = 0;
    for (int dx = -1; dx <= +1; dx++) {
      for (int dy = -1; dy <= +1; dy++) {
        int ox = (n + x + dx) % n; // handle wrap-around
        int oy = (n + y + dy) % n;

        if (curr.at(oy * n + ox) && !(dx == 0 && dy == 0))
          neighbours++;
      }
    }

    if (curr[n * y + x]) {
      // alive
      if (neighbours < 2) {
        return false;
      } else if (neighbours > 3) {
        return false;
      } else {
        return true;
      }
    } else {
      // dead
      if (neighbours == 3) {
        return true;
      } else {
        return false;
      }
    }
  }

};

#endif
