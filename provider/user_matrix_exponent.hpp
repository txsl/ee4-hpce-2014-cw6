#ifndef user_matrix_exponent_hpp
#define user_matrix_exponent_hpp

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

  virtual void Execute(
    puzzler::ILog *log,
    const puzzler::MatrixExponentInput *input,
    puzzler::MatrixExponentOutput *output
  ) const override {
    std::vector<uint32_t> hash(input->steps);

    log->LogVerbose("Setting up A and identity");
    auto A = MatrixCreate(input->n, input->seed);
    auto acc = MakeSpecialMatrix(input->n, A);
    hash[0] = 1;
    if (input->steps < 2) return;
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

};

#endif
