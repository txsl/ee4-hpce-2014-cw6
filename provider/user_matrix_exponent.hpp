#ifndef user_matrix_exponent_hpp
#define user_matrix_exponent_hpp

#include "puzzler/puzzles/matrix_exponent.hpp"

class MatrixExponentProvider
  : public puzzler::MatrixExponentPuzzle
{
public:
  MatrixExponentProvider()
  {}

  static std::vector<uint32_t> MatrixIdentity(unsigned n)
  {
    std::vector<uint32_t> res(n * n, 0);
    for (unsigned i = 0; i < n; i++) {
      res[i * n + i] = 1;
    }
    return res;
  }

  // static std::vector<uint32_t> MatrixMul(unsigned n, std::vector<uint32_t> a, std::vector<uint32_t> b)
  // {
  //   std::vector<uint32_t> res(n * n, 0);
  //   std::vector<uint32_t> res2(n * n, 0);
    
  //   for (unsigned r = 0; r < n; r++) {
  //     for (unsigned i = 0; i < n; i++) {
  //       res2[r * n] = Add(res2[r*n], Mul(a[r * n + i], b[i * n + r]));
  //     }
  //     for (unsigned c = 1; c < n; c++) {
  //       for (unsigned i = 0; i < n; i++) {
  //         res2[r * n + c] = res2[r * n]; // we know this is the same as the above result - no need to Add or multiply again.
  //       }
  //     }
  //   }
  //   return res2;
  // }

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
    // if (input->steps < 2) return;
    hash[1] = acc[0];


// log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
//   dst << "Printing matrix acc";
//   for (unsigned i = 0; i < input->n; i++) {
//     dst << std::endl;
//     for (unsigned j = 0; j < input->n; j++) {
//       dst << acc[i*input->n + j];
//     }
//   }
// });

log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
  dst << "Printing matrix A";
  for (unsigned i = 0; i < input->n; i++) {
    dst << std::endl;
    for (unsigned j = 0; j < input->n; j++) {
      dst << A[i*input->n + j] << " ";
    }
  }
});

log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
  dst << "Printing vec acc" << std::endl;
  for (unsigned i = 0; i < input->n; i++) {
    dst << acc[i]  << std::endl;
  }
});









//     log->LogVerbose("Beginning multiplication ORIGINAL");
//     hash[0] = acc[0];
//     for (unsigned i = 1; i < input->steps; i++) {
//       log->LogDebug("Iteration %d", i);
//       acc = MatrixMul(input->n, acc, A);
//       hash[i] = acc[0];

// log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
//   dst << "Printing matrix acc";
//   for (unsigned i = 0; i < input->n; i++) {
//     dst << std::endl;
//     for (unsigned j = 0; j < input->n; j++) {
//       dst << acc[i*input->n + j] << " ";
//     }
//   }
// });

// log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
//   dst << "Printing matrix hash";
//   dst << std::endl;
//   for (unsigned i = 0; i < input->steps; i++) {
//     dst << hash[i] << " ";
//   }
// });

//     }







    log->LogVerbose("Beginning multiplication - Smart Way");
    for (unsigned i = 2; i < input->steps; i++) {
      log->LogDebug("Iteration %d", i);
      acc = MatrixSpecialMul(input->n, acc, A);
      hash[i] = acc[0];
    }

log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
  dst << "Printing matrix hash";
  dst << std::endl;
  for (unsigned i = 0; i < input->steps; i++) {
    dst << hash[i] << " ";
  }
});

    log->LogVerbose("Done");
    output->hashes = hash;
  }

};

#endif
