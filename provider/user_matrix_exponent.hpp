#ifndef user_matrix_exponent_hpp
#define user_matrix_exponent_hpp

#include "opencl_frame.hpp"
#include "puzzler/puzzles/matrix_exponent.hpp"

class MatrixExponentProvider
  : public puzzler::MatrixExponentPuzzle
{
public:
  MatrixExponentProvider()
  {}

  
  static std::vector<uint32_t> MatrixSpecialMul(unsigned n, std::vector<uint32_t> a, std::vector<uint32_t> b)
  {
    std::vector<uint32_t> res(n, 0);
    
    for (unsigned r = 0; r < n; r++) {
      for (unsigned i = 0; i < n; i++) {
        res[r] = Add(res[r], Mul(a[r], b[i * n + r]));
      }
    }
    return res;
  }

  static std::vector<uint32_t> MakeSpecialMatrix(unsigned n, std::vector<uint32_t> a)
  {
    std::vector<uint32_t> res(n, 0);
    for (unsigned i = 0; i < n; i++) {
      res[i] = Add(a[i*n + i], 0);
    }
    return res;
  }


  void ExecuteWithOpenCl(
    puzzler::ILog *log,
    const puzzler::MatrixExponentInput *input,
    puzzler::MatrixExponentOutput *output
  ) const {
    std::vector<uint32_t> hash(input->steps);
    size_t cbBufferMatrix = input->n*input->n*sizeof(uint32_t);
    size_t cbBufferVector =          input->n*sizeof(uint32_t);

    log->LogVerbose("Setting up A and identity");
    auto A = MatrixCreate(input->n, input->seed);
    auto acc = MakeSpecialMatrix(input->n, A);
    hash[0] = 1;
    if (input->steps < 2) {
      log->LogVerbose("Ending prematurely, as size = 1");
      output->hashes = hash;
      return;
    }
    hash[1] = acc[0];

    puzzler::OpenclHelper opencl(log);
    opencl.selectPlatform().selectDevice().makeContext();
    opencl.build("matrix_exponent_kernel.cl");

    cl::CommandQueue queue(opencl.getContext(), opencl.getDevice());

    cl::Buffer buffCurrMatrix(opencl.getContext(), CL_MEM_READ_WRITE, cbBufferMatrix);
    cl::Buffer buffCurrVector(opencl.getContext(), CL_MEM_READ_WRITE, cbBufferVector);
    cl::Buffer buffNextVector(opencl.getContext(), CL_MEM_READ_WRITE, cbBufferVector);
    cl::NDRange offset(0);
    cl::NDRange globalSize(input->n);
    cl::NDRange localSize = cl::NullRange;

    cl::Kernel kernel(opencl.getProgram(), "matrix_multiply");
    queue.enqueueWriteBuffer(buffCurrMatrix, CL_TRUE, 0, cbBufferMatrix, &A[0], NULL);
    queue.enqueueWriteBuffer(buffCurrVector, CL_TRUE, 0, cbBufferVector, &acc[0], NULL);
    kernel.setArg(1, buffCurrMatrix);

    log->LogVerbose("Beginning multiplication - OpenCL");
    for (unsigned i = 2; i < input->steps; i++) {
      log->LogDebug("Iteration %d", i);
      kernel.setArg(0, buffCurrVector);
      kernel.setArg(2, buffNextVector);

      queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize); //, &kernelDependencies, &evExecutedKernel);
      queue.enqueueBarrier();
      queue.enqueueReadBuffer(buffNextVector, CL_TRUE, 0, sizeof(uint32_t), &hash[i]);

      std::swap(buffCurrVector, buffNextVector);
    }

    log->LogVerbose("Done");
    output->hashes = hash;
  }


  void ExecuteNoOpenCl(
    puzzler::ILog *log,
    const puzzler::MatrixExponentInput *input,
    puzzler::MatrixExponentOutput *output
  ) const {
    std::vector<uint32_t> hash(input->steps);

    log->LogVerbose("Setting up A and identity");
    auto A = MatrixCreate(input->n, input->seed);
    auto acc = MakeSpecialMatrix(input->n, A);
    hash[0] = 1;


    if (input->steps < 2) {
      log->LogVerbose("Ending prematurely, as size = 1");
      output->hashes = hash;
      return;
    }
    hash[1] = acc[0];

    log->LogVerbose("Beginning multiplication - Smart Way");
    for (unsigned i = 2; i < input->steps; i++) {
      log->LogDebug("Iteration %d", i);
      acc = MatrixSpecialMul(input->n, acc, A);
      hash[i] = acc[0];
    }

    log->LogVerbose("Done");
    output->hashes = hash;
  }




  virtual void Execute(
    puzzler::ILog *log,
    const puzzler::MatrixExponentInput *input,
    puzzler::MatrixExponentOutput *output
  ) const override {
    char* s = getenv("HPCE_CL_ENABLE");
    int v = 0;
    if (s!=NULL) {
      v = atoi(getenv("HPCE_CL_ENABLE"));
    }

    if(v==1) {
      log->LogInfo("Running with Open CL");
      ExecuteWithOpenCl(log, input, output);
    } else {
      log->LogInfo("Running without Open CL");
      ExecuteNoOpenCl(log, input, output);
    }
    
  }


};

#endif
