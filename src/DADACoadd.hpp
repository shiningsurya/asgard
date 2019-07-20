#pragma once
#include <asgard.hpp>
#include <FilterbankSink.hpp>
// DADA
#include <PsrDADA.hpp>
// xRFI
#include "xRFI.hpp"
using exParam = excision::excisionParams;
// heimdall
constexpr std::string candodir = "/mnt/ssd/cands"
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
    // data voting
    bool vote;
    unsigned int numants_votes;
    // DADA
    key_t key_in, key_out1, key_out2, key_out;
    bool filout;
    bool inswitch;
    bool keepgoing;
    bool incomplete;
    // three important numbers
    timeslice nsamps;
    int nchans, nbits;
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
    struct Header dHead;
    // Output DADA objects
    PsrDADA dadaout;
    bool one_two; // true if dadaout1 or dadaout2
    // Filterbank Writer
    FilterbankSink fbout;
    // Heimdall
    Shell heimdall_sh;
    unsigned int gpu_id;
    // debug
    unsigned int numobs;
    // xRFI
    struct excision::excisionParams eparam;
    excision::xRFI xrfi;
    // coadder
    void coadder() {
      numobs = 0;
      // connect to out buffer in root
      if(world.rank() == world_root) {
        key_out = one_two ? key_out1 : key_out2;
        dadaout = PsrDADA(key_out, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadaout.log");
        one_two = not one_two;
      }
      while(++numobs) // for every observation
      {
        // Barrier to have all the nodes start new observation at the same time
        world.barrier();
        std::cerr << "DADACoadd::Coadder New Obs numobs=" << numobs << " key=" << key_out << std::endl; 
        PsrDADA dadain(key_in, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadain.log");
        keepgoing = false;
        incomplete = false;
        running_index = 0;
        dadain.ReadLock(true);
        if(world.rank() == world_root) dadaout.WriteLock(true);
        // assume for the beginning everyone is participating
        vote = true;
        addcomm = std::move(world.split(true));
        while (  keepgoing  ||  dadain.ReadHeader()  ) // for stretch of observation 
        {
          // READING
          std::cerr << "DADACoadd::READING rank=" << world.rank() << " ridx=" << running_index; 
          std::cerr << " key=" << key_out << std::endl;
          read_chunk = dadain.ReadData(data_f, data_b);
          // if Read header for the first time
          if(!running_index) dHead = std::move(dadain.GetHeader());
          if(!running_index) dadain.PrintHeader();
          if( read_chunk == -1 ) {
            // fill zeros because read failed
            // EOD read fail
            std::fill(data_f, data_f + sample_chunk, 0.0f);
            vote = false;
            // reset counter
            keepgoing = false;
            // begin new observation
            break;
          }
          else if( read_chunk < bytes_chunk) {
            // fill zeros at the end
            std::fill(data_f + sample_chunk - (read_chunk * 8 / nbits), data_f + sample_chunk, 0.0f);
            vote = true;
            // reset counter
            keepgoing = false;
            incomplete = true;
          }
          else if( read_chunk == bytes_chunk) {
            // perfect world case
            vote = true;
            keepgoing = true;
            incomplete = false;
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
          // RFI excision -- level 1
          std::cout << "DADACoadd::xRFI Level 1 Filter=" << eparam.filter << std::endl;
          xrfi.Excise(data_f, eparam.filter);
#if 0
          // like # data reads in dadain == # writes in dadaout
          // ^ this seems like super stringent
#endif
          // actual MPI coadd call
          mpi::reduce(addcomm, data_f, sample_chunk, o_data_f, std::plus<float>(), addcomm_root);
          // WRITING
          if(addcomm.rank() == addcomm_root) {
            // set Header
            if(!running_index) {
              // write out Header
              std::cerr << "DADACoadd::WRITING HEADER";
              std::cerr << " key=" << key_out;
              std::cerr << " rank=" << addcomm.rank() << std::endl;
              dHead.stationid = 0;
              dadaout.SetHeader(dHead);
              dadaout.WriteHeader();
              // Filterbank Initialize
              if(filout) fbout.Initialize(dHead);
            }
            std::cerr << "DADACoadd::WRITING DATA";
            std::cerr << " key=" << key_out;
            std::cerr << " rank=" << addcomm.rank() << std::endl;
            // RFI excision -- level 2
            std::cout << "DADACoadd::xRFI Level 2 Filter=" << eparam.filter << std::endl;
            xrfi.Excise(o_data_f, eparam.filter);
            // write data
            dadaout.WriteData(o_data_f, o_data_b, addcomm.size());
            // Filterbankwrite
            if(filout) {
              fbout.Data(o_data_b, read_chunk); 
            }
            // heimdall
            if(running_index == 2) {
              // only running heimdall after two writes
              // args --> GPU_ID OUTDIR STATION_ID DADA_KEY
              gpu_id = one_two ? 0 : 1;
              heimdall_sh.ReadRun(true, gpu_id, candodir, dHead.station_id, key_out)
            }
            std::cerr << "DADACoadd::Running Index=" << running_index;
            std::cerr << " key=" << key_out;
            std::cerr << " rank=" << world.rank() << std::endl;
          }
          // new communicator
          addcomm = std::move(addcomm.split(keepgoing&&!incomplete));
          // root resolution and broadcasting
          if(world.rank() == world_root) {
            addcomm_root = addcomm.rank();
          }
          mpi::broadcast(world, addcomm_root, world_root);
          running_index++;
          if(incomplete) break;
        } // for stretch of observation
        dadain.ReadLock(false);
        if(addcomm.rank() == addcomm_root) {
          dadaout.WriteLock(false);
          if(filout)
            fbout.Close();
        }
      } // for every observation
      // dada objects should be destroyed here
      // dadain goes out of scope
      // dadaout dtor is destroyed while mov_assign is called
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
        one_two(true),
        filout(filout_),
        nsamps(nsamps_),
        nchans(nchans_),
        nbits(nbits_),
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
            // initialize shell
            heimdall_sh.SetFmt("/home/vlite-master/mtk/bin/heimdall -nsamps_gulp 30720 -gpu_id %d -dm 2 1000 -boxcar_max 64 -output_dir %s -group_output -zap_chans 0 190 -zap_chans 3900 4096 -beam %d -k %x -coincidencer vlite-nrl:27555 -V &> /mnt/ssd/cands/heimdall_log.asc");
            // args --> GPU_ID OUTDIR STATION_ID DADA_KEY
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
