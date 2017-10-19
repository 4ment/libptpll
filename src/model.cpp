#include "model.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <libpll/pll.h>

#include "pll_util.hpp"

namespace pt { namespace pll {

/// @brief Partition string according to delimiter.
std::vector<std::string> ssplit(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

Model ParseRaxmlInfo(const std::string& path, size_t rate_categories)
{
  std::ifstream file(path);
  std::string read;
  std::string contents;

  while (std::getline(file, read)) {
    contents += read;
    contents.push_back('\n');
  }

  // initialize the array of base frequencies
  std::size_t pos1 = contents.find("frequencies: ");
  std::size_t pos2 = contents.find('\n', pos1);
  std::string sstr = contents.substr(pos1 + 13, pos2 - pos1 - 13);

  std::vector<std::string> freqvector = ssplit(sstr, ' ');
  std::vector<double> frequencies(freqvector.size());

  for (unsigned int i = 0; i < freqvector.size(); i++)
    frequencies[i] = std::stod(freqvector.at(i));

  // initialize the array of subst_params
  pos1 = contents.find("ac ag at cg ct gt:");
  pos2 = contents.find('\n', pos1);
  sstr = contents.substr(pos1 + 19, pos2 - pos1 - 19);
  std::vector<std::string> ratevector = ssplit(sstr, ' ');
  std::vector<double> subst_params(ratevector.size());

  for (unsigned int i = 0; i < ratevector.size(); i++)
    subst_params[i] = std::stod(ratevector.at(i));
  if (ratevector.size() != (((freqvector.size()) * freqvector.size() - 4) / 2)) {
    throw std::invalid_argument("Wrong number of rate values.");
  }

  // initialize the alpha value
  pos1 = contents.find("alpha[0]: ");
  pos2 = contents.find(' ', pos1 + 10);
  sstr = contents.substr(pos1 + 10, pos2 - pos1 - 10);
  double alpha = stod(sstr);

  std::vector<double> category_rates(rate_categories);

  pll_compute_gamma_cats(alpha,
                         category_rates.size(),
                         category_rates.data(),
                         PLL_GAMMA_RATES_MEAN);

  // initialize the model name
  pos1 = contents.find("Substitution Matrix: ");
  pos2 = contents.find('\n', pos1);
  sstr = contents.substr(pos1 + 21, pos2 - pos1 - 21);
  std::string model_name = sstr;

  Model model{model_name, frequencies, subst_params, category_rates};
  return model;
}

} } // namespace pt::pll