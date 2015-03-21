#ifndef user_string_search_hpp
#define user_string_search_hpp

#include <random>

#include "puzzler/core/puzzle.hpp"
#include "puzzler/puzzles/string_search.hpp"

class StringSearchProvider
  : public puzzler::StringSearchPuzzle
{
public:
  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::StringSearchInput *input,
		       puzzler::StringSearchOutput *output
		       ) const override
  {
    std::vector<uint32_t> histogram(input->patterns.size(), 0);

    std::string data = MakeString(input->stringLength, input->seed);

    int K = 1;
    v = getenv("HPCE_CHUNKSIZE_K");
    if (v == NULL) {
      log->LogInfo("No HPCE_CHUNKSIZE_K envrionment variable found");
    } else {
      K = atoi(v);
      log->LogInfo("HPCE_CHUNKSIZE_K environment variable found as %i", K);
    } 

    typedef tbb::blocked_range<unsigned> my_range_t;
    my_range_t range(0, input->patterns.size(), K);

    unsigned i = 0;
    while (i < input->stringLength) {

      std::vector <unsigned> lens(input->patterns.size(), 0);


      for (unsigned p = 0; p < input->patterns.size(); p++) {
        lens[p] = Matches(data, i, input->patterns[p]);
      }

      for (unsigned p = 0; p < input->patterns.size(); p++) {
        if (lens[p] > 0) {
          log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
            dst << "  Found " << input->patterns.at(p) << " at offset " << i << ", match=" << data.substr(i, lens[p]);
          });
          histogram[p]++;
          i += lens[p] - 1;
          break;
        }
      }
      ++i;
    }

    for (unsigned i = 0; i < histogram.size(); i++) {
      log->Log(puzzler::Log_Debug, [&](std::ostream & dst) {
        dst << input->patterns[i].c_str() << " : " << histogram[i];
      });
    }


    output->occurences = histogram;
  }

};

#endif
