#pragma once
#include <asgard.hpp>
#include <FilterbankSink.hpp>
// DADA
#include <PsrDADA.hpp>
// xRFI
#include "xRFI.hpp"
using exParam = excision::excisionParams;
// MPI
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/operations.hpp>
// for max precision printing
#include <iomanip>
#include <limits>
#include <set>

namespace mpi = boost::mpi;
class DADACoadd  {
  private:
    // MPI
    int world_root;
    mpi::environment env;
    mpi::communicator world;
    uint64_t running_index;
    unsigned int numants, yes, no;
    bool participate;
    // DADA
    key_t key_in, key_out1, key_out2, key_out;
    unsigned int dkey_out1, dkey_out2, dkey_out;
    bool filout;
    int incomplete, eod;
    // three important numbers
    timeslice nsamps;
    int nchans, nbits;
    double ctime, rtime;
    int double_maxprecision;
    std::vector<double> vec_ctime;
    int fb_nbits;
    // stride numbers
    timeslice sample_stride;
    // bytes chunk
    timeslice bytes_chunk;
    // sample chunk
    timeslice sample_chunk;
    timeslice read_chunk;
    // Buffers
    PtrFloat data_f;
    PtrFloat o_data_f, zero_data_f;
    PtrByte data_b;
    PtrByte o_data_b;
    struct Header dHead, oldheader;
    // Output DADA objects
    PsrDADA dadaout;
    // Filterbank Writer
    FilterbankSink fbout;
    // debug
    unsigned int numobs;
    // xRFI
    struct excision::excisionParams eparam;
    excision::xRFI xrfi;
    // coadder
    void coadder() {
      numobs = 0;
      key_out = key_out1;
      while(++numobs) // for every observation
      {
        // connect to out buffer in root
        if(world.rank() == world_root) {
          dadaout = PsrDADA(key_out, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadaout.log");
        }
        std::cerr << "DADACoadd::Coadder New Obs numobs=" << numobs << " key=" << std::hex << key_out << std::endl; 
        PsrDADA dadain(key_in, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadain.log");
        // default to true for logic to go into read-header
        eod = 1;
        incomplete = 1;
        running_index = 0;
        dadain.ReadLock(true);
        if(world.rank() == world_root) dadaout.WriteLock(true);
        // assume for the beginning everyone is participating
        // addcomm = std::move(world.split(true));
        // all nodes must be in sync
        oldheader = dHead;
        while (  (!eod && !incomplete) ||  dadain.ReadHeader()  ) // for stretch of observation 
        {
          // READING
          std::cerr << "DADACoadd::READING rank=" << world.rank() << " ridx=" << running_index; 
          std::cerr << " key=" << std::hex << key_out << std::endl;
          read_chunk = dadain.ReadData(data_f, data_b);
          // if Read header for the first time
          if(!running_index) {
            dHead = std::move(dadain.GetHeader());
            dadain.PrintHeader();
          }
          if( read_chunk == -1 ) {
            // fill zeros because read failed
            // EOD read fail
            std::fill(data_f, data_f + sample_chunk, 0.0f);
            // log
            std::cerr << "DADACoadd::EOD node=" << env.processor_name() << " ridx=" << running_index; 
            eod = 1;
            incomplete = 0;
          }
          else if( read_chunk < bytes_chunk) {
            // fill zeros at the end
            std::fill(data_f + sample_chunk - (read_chunk * 8 / nbits), data_f + sample_chunk, 0.0f);
            // reset counter
            incomplete = 1;
            eod = 0;
            // log
            std::cerr << "DADACoadd::Incomplete node=" << env.processor_name() << " ridx=" << running_index; 
          }
          else if( read_chunk == bytes_chunk) {
            // perfect world case
            eod = 0;
            incomplete = 0;
          }
          // sanity checks like local indices are same
          ctime = dHead.tstart + (running_index * nsamps * dHead.tsamp * 1E-6f / 86400.f);
          std::cout << "DADACoadd::Coadder ctime=" << std::dec << std::setprecision(double_maxprecision)<< ctime;
          std::cout << " rank=" << std::setprecision(6) << world.rank() << std::endl;
          // BARRIER
          mpi::gather(world, ctime, vec_ctime, world_root);
          if (world.rank() == world_root) {
            std::set<double> rtime (vec_ctime.begin(), vec_ctime.end());
            if( rtime.size() == 1 ) {
              // reset vec_rindex for future use
              vec_ctime.clear();
              participate = true;
            }
            else {
              // loose hell
              std::cerr << "DADACoadd::syncheck #1 failed!" << std::endl;
              std::cerr << "Abort!" << std::endl;
              int i_ = 0;
              for (auto k_ : rtime) {
                std::cerr << "\trtime["<< std::dec << i_++ << "]="; 
                std::cerr << std::setprecision(double_maxprecision) << k_ << std::endl;
                std::cerr << std::setprecision(6);
              }
              participate = false;
            }
          }
          // RFI excision -- level 1
          xrfi.Excise(data_f, eparam.filter);
          // actual MPI coadd call -- BARRIER
          rtime = ctime;
          mpi::broadcast (world, rtime, world_root);
          participate = ctime == rtime;
          if (!participate) {
            std::cerr << "DADACoadd::Coadder not participating" << std::endl;
            std::cerr << "My   ctime=" << std::setprecision(double_maxprecision) << ctime << std::endl;
            std::cerr << "Root ctime=" << std::setprecision(double_maxprecision) << rtime << std::endl;
          }
          mpi::reduce(world, participate ? yes : no, numants, std::plus<unsigned int>(), world_root);
          mpi::reduce(world, participate ? data_f : zero_data_f, sample_chunk, o_data_f, std::plus<float>(), world_root);
          // WRITING
          if(world.rank() == world_root) {
            // set Header
            if(!running_index) {
              // write out Header
              std::cerr << "DADACoadd::WRITING HEADER";
              std::cerr << " key=" << std::hex << key_out;
              std::cerr << " rank=" << std::dec << world.rank() << std::endl;
              dHead.stationid = 99;
              dadaout.SetHeader(dHead);
              dadaout.WriteHeader();
              // Filterbank Initialize
              if(filout) fbout.Initialize(dHead, fb_nbits);
            }
            std::cerr << "DADACoadd::WRITING DATA";
            std::cerr << " key=" << std::hex << key_out;
            std::cerr << " rank=" << std::dec << world.rank() << std::endl;
            // RFI excision -- level 2
            xrfi.Excise(o_data_f, eparam.filter);
            // write data
            dadaout.WriteData(o_data_f, o_data_b, numants);
            // Filterbankwrite
            if(filout) {
              dadaout.Redigitize(o_data_f, o_data_b, fb_nbits, numants);
              fbout.Data(o_data_b, read_chunk * fb_nbits / nbits); 
            }
            std::cerr << "DADACoadd::Running Index=" << running_index;
            std::cerr << " key=" << std::hex << key_out;
            std::cerr << " rank=" << std::dec << world.rank() << std::endl;
          }
          if( eod || incomplete ) {
            // begin new observation
            break;
          }
          running_index++;
        } // for stretch of observation
        dadain.ReadLock(false);
        if(world.rank() == world_root) {
          dadaout.WriteLock(false);
          dadaout.~PsrDADA();
          if(filout)
            fbout.Close();
        }
      } // for every observation
      // dada objects should be destroyed here
      // dadain goes out of scope
      // dadaout dtor is explicitly called
      std::cout << "DADACoadd::COADDER exiting numobs=" << numobs << std::endl;
    }
  public:
    DADACoadd(key_t key_in_, 
        key_t key_out_, 
        bool filout_, 
        timeslice nsamps_,
        int nchans_,
        int nbits_,
        const exParam& _xp,
        int root_) 
      :
        key_in(key_in_), 
        key_out1(key_out_),
        key_out2(key_out_ + 2),
        filout(filout_),
        nsamps(nsamps_),
        nchans(nchans_),
        nbits(nbits_),
        fb_nbits(2),
        eparam(_xp),
        xrfi(eparam, nsamps, nchans),
        running_index(0),
        ctime(0.0),
        yes(1), no(0),
        world_root(root_) {
          // common ground work
          sample_stride = nchans * nbits / 8;
          bytes_chunk = nsamps * nchans * nbits / 8;
          sample_chunk = nsamps * nchans;
          // initialize buffers
          data_b = new unsigned char[bytes_chunk];
          data_f = new float[sample_chunk];
          zero_data_f = new float[sample_chunk]();
          o_data_b = nullptr;
          o_data_f = nullptr;
          // ground work in root
          if(world.rank() == world_root) {
            // initialize output buffers
            o_data_b = new unsigned char[bytes_chunk];
            o_data_f = new float[sample_chunk];
          }
          // one time logging
          std::cout << "DADACoadd::xRFI Level 1 Filter=" << eparam.filter << std::endl;
          std::cout << "DADACoadd::xRFI Level 2 Filter=" << eparam.filter << std::endl;
          // max-precision
          double_maxprecision = std::numeric_limits<double>::digits10 + 1;
        }
    ~DADACoadd() {
      delete[] data_b;
      delete[] data_f;
      delete[] zero_data_f;
      if(o_data_b != nullptr) delete[] o_data_b;
      if(o_data_f != nullptr) delete[] o_data_f;
    }
    void Work() {
      coadder();
    }
};
