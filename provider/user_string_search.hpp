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
    if (input->stringLength > 1200000 || input->stringLength < 900) {
      ReferenceExecute(log, input, output);
      return;
    }
    typedef tbb::blocked_range<unsigned> my_range_t;
    std::vector<uint32_t> histogram(input->patterns.size(), 0);
    std::string data = MakeString(input->stringLength, input->seed);

    my_range_t range(0, input->patterns.size(), 16);

    std::vector <unsigned> lens(input->patterns.size(), 0);
    unsigned i = 0;

    auto impl = [&](const my_range_t &chunk){
      for (unsigned p = chunk.begin(); p < chunk.end(); p++) {
        lens[p] = Matches(data, i, input->patterns[p]);
      }
    };

    while (i < input->stringLength) {
      // thanks http://stackoverflow.com/questions/8848575/fastest-way-to-reset-every-value-of-stdvectorint-to-0
      std::fill(lens.begin(), lens.end(), 0);
      tbb::parallel_for(range, impl, tbb::simple_partitioner());
      
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
