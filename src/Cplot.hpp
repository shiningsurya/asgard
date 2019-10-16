#include "asgard.hpp"
#include "Filterbank.hpp"
#include "Candidate.hpp"
#include "Operations.hpp"
#include "FilterbankCandidate.hpp" 
#include "Plotter.hpp"

class Cplot : protected Plotter {
  private:

  public:
    Cplot(std::string fn) : Plotter(fn) {
      cpgpap (0.0,0.618); //10.0, width and aspect ratio
      fac = 1e-2f;
      txtheight = -1.5 * charh;
    }
    void AddSNDM () {

    }

};
