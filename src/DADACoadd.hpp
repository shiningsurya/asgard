#pragma once
#include <asgard.hpp>
#include <FilterbankSink.hpp>
// DADA
#include <PsrDADA.hpp>
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
	key_t key_in, key_out;
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
	// Output DADA object
	PsrDADA dadaout;
	// Filterbank Writer
	FilterbankSink fbout;
	// debug
	int numobs;
	// coadder
	void coadder() {
	 numobs = 0;
	 // connect to out buffer in root
	 if(world.rank() == world_root) {
		dadaout = PsrDADA(key_out, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadaout.log");
	 }
	 while(++numobs) // for every observation
	 {
		// Barrier to have all the nodes start new observation at the same time
		world.barrier();
		std::cerr << "DADACoadd::COADDER Beginning new observation"; 
		std::cerr << " rank=" << world.rank() << std::endl;
		PsrDADA dadain(key_in, nsamps, nchans, nbits, "/home/vlite-master/surya/logs/dadain.log");
		keepgoing = false;
		incomplete = false;
		running_index = 0;
		dadain.ReadLock(true);
		if(world.rank() == world_root) dadaout.WriteLock(true);
		// assume for the beginning everyone is participating
		addcomm = std::move(world.split(true));
		while (  keepgoing  ||  dadain.ReadHeader()  ) // for stretch of observation 
		{
		 // READING
		 std::cerr << "DADACoadd::READING rank=" << world.rank() << std::endl;
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
		 }
		 // sanity checks like local indices are same
		 mpi::gather(addcomm, running_index, vec_rindex, addcomm_root);
		 if(! std::adjacent_find( vec_rindex.cbegin(), vec_rindex.cend(), std::not_equal_to<uint64_t>() )) {
				 std::cerr << "DADACoadd::syncheck #2 failed!" << std::endl;
				 std::cerr << "DADACoadd::syncheck #2 MAJOR ERROR!" << std::endl;
				 std::cerr << "Abort! Abort! Abort!" << std::endl;
		 }
		 else {
		 		 // reset vec_rindex for future use
		 		 vec_rindex.clear();
		 }
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
			 std::cerr << " rank=" << addcomm.rank() << std::endl;
			 dHead.stationid = 0;
			 dadaout.SetHeader(dHead);
			 dadaout.WriteHeader();
			 // Filterbank Initialize
			 if(filout) fbout.Initialize(dHead);
			}
			std::cerr << "DADACoadd::WRITING DATA";
			std::cerr << " rank=" << addcomm.rank() << std::endl;
			// write data
			dadaout.WriteData(o_data_f, o_data_b, addcomm.size());
			// Filterbankwrite
			if(filout) {
			 fbout.Data(o_data_b, read_chunk); 
			}
			std::cerr << "DADACoadd::Running Index=" << running_index++ ;
			std::cerr << " rank=" << world.rank() << std::endl;
		 }
		 // new communicator
		 addcomm = std::move(addcomm.split(vote|incomplete));
		 // root resolution and broadcasting
		 if(world.rank() == world_root) {
			addcomm_root = addcomm.rank();
		 }
		 mpi::broadcast(world, addcomm_root, world_root);
		 if(incomplete) break;
		} // for stretch of observation
		dadain.ReadLock(false);
		if(addcomm.rank() == addcomm_root) {
		 dadaout.WriteLock(false);
		 fbout.Close();
		}
	 } // for every observation
	 // dada objects should be destroyed here
	 // dadain goes out of scope
	 // dadaout dtor is destroyed while mov_assign is called
	}
 public:
	DADACoadd(key_t key_in_, 
		key_t key_out_, 
		bool filout_, 
		timeslice nsamps_,
		int nchans_,
		int nbits_,
		int root_) 
	 :
		key_in(key_in_), 
		key_out(key_out_),
		filout(filout_),
		nsamps(nsamps_),
		nchans(nchans_),
		nbits(nbits_),
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
			// FilterbankWriter in root
#if 0
			if(filout) boundcheck = fbw.Initialize();
#endif 
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
