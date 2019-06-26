#pragma once
#include <asgard.hpp>
#include <Filterbank.hpp>
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
				// Filterbankout
				FilterbankWriter fbw;
				timeslice boundcheck;
				// MPI
				int world_root, addcomm_root;
				mpi::environment env;
				mpi::communicator world;
				mpi::communicator addcomm;
				uint64_t running_index;
				// data voting
				bool vote;
				unsigned int numants_votes;
				// DADA
				key_t key_in, key_out;
				bool filout;
				bool inswitch;
				bool keepgoing;
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
				struct DADAHeader dHead;
				// Output DADA object
				PsrDADA dadaout;
				// coadder
				void coadder() {
					while(true) // for every observation
					{
					std::cerr << "DADACoadd::COADDER Beginning new observation" << std::endl;
					PsrDADA dadain(key_in, nsamps, nchans, nbits);
					keepgoing = false;
					dadain.ReadLock(true);
					if(world.rank() == world_root) {
					 dadaout = PsrDADA(key_out, nsamps, nchans, nbits);
					 dadaout.WriteLock(true);
					}
					 while (  keepgoing  ||  dadain.ReadHeader()  ) // for stretch of observation 
					 {
						 // READING
						 std::cerr << "DADACoadd::READING" << std::endl;
						 read_chunk = dadain.ReadData(data_f, data_b);
						 // if Read header for the first time
						 if(!keepgoing) dHead = std::move(dadain.GetHeader());
						 if(!keepgoing) dadain.PrintHeader();
						 if( read_chunk == -1 ) {
							 // fill zeros because read failed
							 // EOD read fail
							 std::fill(data_f, data_f + sample_chunk, 0.0f);
							 vote = false;
							 // reset counter
							 keepgoing = false;
							 running_index = 0;
							 // go back to LOOPING
							 break;
						 }
						 else if( read_chunk < bytes_chunk) {
							 // fill zeros at the end
							 std::fill(data_f + sample_chunk - (read_chunk/sample_stride), data_f + sample_chunk, 0.0f);
							 vote = true;
							 // reset counter
							 keepgoing = false;
							 running_index = 0;
						 }
						 else if( read_chunk == bytes_chunk) {
							 // perfect world case
							 vote = true;
							 keepgoing = true;
						 }
						 // VOTING
						 addcomm = std::move(world.split(vote));
						 // root resolution and broadcasting
						 if(world.rank() == world_root) {
							 addcomm_root = addcomm.rank();
						 }
						 mpi::broadcast(world, addcomm_root, world_root);
						 // actual MPI coadd call
						 mpi::reduce(addcomm, data_f, sample_chunk, o_data_f, std::plus<float>(), addcomm_root);
						 // WRITING
						 if(addcomm.rank() == addcomm_root) {
								 // set Header
							 dadaout.SetHeader(dHead);
							 if(!running_index) {
									 // should I WriteLock(true) here?
									 // dadaout.WriteLock(true);
									 // write out Header
									 dadaout.WriteHeader();
									 std::cerr << "DADACoadd::WRITING HEADER" << std::endl;
								 }
								 std::cerr << "DADACoadd::WRITING DATA" << std::endl;
							 // write data
							 dadaout.WriteData(o_data_f, o_data_b, addcomm.size());
							 // Filterbankwrite
							 if(filout) {
								 // write filterbank 
								 fbw.WriteFBdata(o_data_f, boundcheck, sample_chunk, addcomm.size());
								 // incrementing as in serial
								 // NB see comments in serial code
								 boundcheck += (sample_chunk * nbits / 8);
							 }
							 // should I WriteLock(false) here?
							 // dadaout.WriteLock(false);
							 running_index++;
						 }
					 } // for stretch of observation
					 dadain.ReadLock(false);
					 if(world.rank() == world_root) dadaout.WriteLock(false);
					} // for every observation
					// dada objects should be destroyed here
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
