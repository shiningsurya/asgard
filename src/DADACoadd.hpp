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
namespace mpi = boost::mpi;
class DADACoadd  {
  private:
    // MPI
    int world_root, addcomm_root;
    mpi::environment env;
    mpi::communicator world;
    mpi::communicator addcomm;
    uint64_t running_index;
    std::vector<uint64_t> vec_rindex;
    // DADA
    key_t key_in, key_out1, key_out2, key_out;
    unsigned int dkey_out1, dkey_out2, dkey_out;
    bool filout;
    int incomplete;
    int eod;
    int red_eod, red_incomplete;
    bool force_iteration;
    // three important numbers
    timeslice nsamps;
    int nchans, nbits;
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
    PtrFloat o_data_f;
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
        // Barrier to have all the nodes start new observation at the same time
        world.barrier();
        std::cerr << "DADACoadd::Coadder New Obs numobs=" << numobs << " key=" << std::hex << key_out << std::endl; 
        PsrDADA dadain(key_in, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadain.log");
        // default to true for logic to go into read-header
        eod = 1;
        incomplete = 1;
        running_index = 0;
        force_iteration = false;
        dadain.ReadLock(true);
        if(world.rank() == world_root) dadaout.WriteLock(true);
        // assume for the beginning everyone is participating
        addcomm = std::move(world.split(true));
        oldheader = dadain.GetHeader();
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
          mpi::gather(addcomm, running_index, vec_rindex, addcomm_root);
          uint64_t ridx = running_index;
          if(std::all_of( vec_rindex.cbegin(), vec_rindex.cend(), [&ridx] (uint64_t i) { return i == ridx;})) {
            // reset vec_rindex for future use
            vec_rindex.clear();
          }
          else {
            std::cerr << "DADACoadd::syncheck #2 failed!" << std::endl;
            std::cerr << "DADACoadd::syncheck #2 MAJOR ERROR!" << std::endl;
            std::cerr << "Abort! Abort! Abort!" << std::endl;
          }
          // sanity checks -- incomplete and eod
          mpi::reduce(addcomm, incomplete, red_incomplete,std::plus<int>(), addcomm_root);
          mpi::reduce(addcomm, eod, red_eod, std::plus<int>(), addcomm_root);
          if(addcomm.rank() == addcomm_root) {
            // create accumulates
            if(red_eod != 0 || red_eod != addcomm.size()) {
              // eod is not uniform
              std::cerr << "DADACoadd::EOD out of sync eod=" << std::dec << red_eod  << std::endl;
              // EOD out of sync is already taken care of
              // a new communicator is created 
            }
            if(red_incomplete != 0 || red_incomplete != addcomm.size()) {
              // incomplete is not uniform
              std::cerr << "DADACoadd::INCOMPLETE out of sync incomplete=" << std::dec << red_incomplete << std::endl;
              // This is very dicey. This is mysterious. 
              // Current approach is conservative
              //force_iteration = true;
            }
            else {
              force_iteration = false;
            }
          }
          mpi::broadcast(addcomm, force_iteration, addcomm_root);
#if 0
          // gather  keepgoing
          // keepgoing = false if EOD || INCOMPLETE
          // incomplete = true if INCOMPLETE
          // eod = true if EOD
          // -------------------------------------------------------
          // like # data reads in dadain == # writes in dadaout
          // ^ this seems like super stringent
          // sanity checks like local indices are same
          mpi::gather(addcomm, running_index, vec_rindex, addcomm_root);
          uint64_t ridx = running_index;
          if(std::all_of( vec_rindex.cbegin(), vec_rindex.cend(), [&ridx] (uint64_t i) { return i == ridx;})) {
            // reset vec_rindex for future use
            vec_rindex.clear();
          }
          else {
            std::cerr << "DADACoadd::syncheck #2 failed!" << std::endl;
            std::cerr << "DADACoadd::syncheck #2 MAJOR ERROR!" << std::endl;
            std::cerr << "Abort! Abort! Abort!" << std::endl;
          }
#endif
          // RFI excision -- level 1
          std::cout << "DADACoadd::xRFI Level 1 Filter=" << eparam.filter << std::endl;
          xrfi.Excise(data_f, eparam.filter);
          // actual MPI coadd call
          mpi::reduce(addcomm, data_f, sample_chunk, o_data_f, std::plus<float>(), addcomm_root);
          // WRITING
          if(addcomm.rank() == addcomm_root) {
            // set Header
            if(!running_index) {
              // write out Header
              std::cerr << "DADACoadd::WRITING HEADER";
              std::cerr << " key=" << std::hex << key_out;
              std::cerr << " rank=" << std::dec << addcomm.rank() << std::endl;
              dHead.stationid = 99;
              dadaout.SetHeader(dHead);
              dadaout.WriteHeader();
              // Filterbank Initialize
              if(filout) fbout.Initialize(dHead, fb_nbits);
            }
            std::cerr << "DADACoadd::WRITING DATA";
            std::cerr << " key=" << std::hex << key_out;
            std::cerr << " rank=" << std::dec << addcomm.rank() << std::endl;
            // RFI excision -- level 2
            std::cout << "DADACoadd::xRFI Level 2 Filter=" << eparam.filter << std::endl;
            xrfi.Excise(o_data_f, eparam.filter);
            // write data
            dadaout.WriteData(o_data_f, o_data_b, addcomm.size());
            // Filterbankwrite
            if(filout) {
              dadaout.Redigitize(o_data_f, o_data_b, fb_nbits, addcomm.size());
              fbout.Data(o_data_b, read_chunk * fb_nbits / nbits); 
            }
            std::cerr << "DADACoadd::Running Index=" << running_index;
            std::cerr << " key=" << std::hex << key_out;
            std::cerr << " rank=" << std::dec << world.rank() << std::endl;
          }
          // new communicator
          addcomm = std::move(addcomm.split(!eod||!incomplete));
          // root resolution and broadcasting
          if(world.rank() == world_root) {
            addcomm_root = addcomm.rank();
          }
          mpi::broadcast(world, addcomm_root, world_root);
          if( (eod || incomplete) && !force_iteration) {
            // begin new observation
            break;
          }
          running_index++;
        } // for stretch of observation
        dadain.ReadLock(false);
        if(addcomm.rank() == addcomm_root) {
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
        world_root(root_) {
          // common ground work
          sample_stride = nchans * nbits / 8;
          bytes_chunk = nsamps * nchans * nbits / 8;
          sample_chunk = nsamps * nchans;
          // initialize buffers
          data_b = new unsigned char[bytes_chunk];
          data_f = new float[sample_chunk];
          o_data_b = nullptr;
          o_data_f = nullptr;
          // ground work in root
          if(world.rank() == world_root) {
            // initialize output buffers
            o_data_b = new unsigned char[bytes_chunk];
            o_data_f = new float[sample_chunk];
          }
        }
    ~DADACoadd() {
      delete[] data_b;
      delete[] data_f;
      if(o_data_b != nullptr) delete[] o_data_b;
      if(o_data_f != nullptr) delete[] o_data_f;
    }
    void Work() {
      coadder();
    }
};
