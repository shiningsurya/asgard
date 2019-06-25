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
				key_t key_in1, key_in2, key_out;
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
				// DADA Objects
				PsrDADA dadain1, dadain2, dadaout;
				struct DADAHeader dHead;
				// coadder
				void coadder() {
					keepgoing = false;
					// Locks were here. See Rationale in PsrDADA
					// run indefinitely
					// blocks on inswitch dadain Read Header
					std::cout << "DADACoadd::LOOPING" << std::endl;
					while( 
							( keepgoing )
							||
							( inswitch   && dadain1.ReadHeader() )
							||
							( !inswitch  && dadain2.ReadHeader() )
							) {
						/****************************************************************************
						 * Post MTK email 6162019
						 * Filterbank data alternates between two DADA buffers.
						 * > dadain1, dadain2
						 * Work loop MO
						 * > dadain1 -> dadain2 -> dadain1
						 * At any time, all the data in a DADA buffer belongs to one observation.
						 * > by design
						 * Flexibility in nodes participating by zero filling
						 * > if read failed or read incomplete
						 * numant computation done by voting
						 * OR
						 * it is communicator size since collective operation is run over a sub-group
						 * --------------------------------------------------------------------------
						 *  One antenna, one process policy
						 * --------------------------------------------------------------------------
						 * *************************************************************************/
						// READING
						std::cerr << "DADACoadd::READING" << std::endl;
						if( inswitch ){
							read_chunk = dadain1.ReadData(data_f, data_b);
							// if Read header for the first time
						    if(!keepgoing) dHead = std::move(dadain1.GetHeader());
						    if(!keepgoing) dadain1.PrintHeader();
						}
						else {
							read_chunk = dadain2.ReadData(data_f, data_b);
							// if Read header for the first time
						    if(!keepgoing) dHead = std::move(dadain2.GetHeader());
						    if(!keepgoing) dadain2.PrintHeader();
						}
						// SWITCHING
						if( read_chunk == -1 ) {
							// fill zeros because read failed
							// EOD read fail
							std::fill(data_f, data_f + sample_chunk, 0.0f);
							vote = false;
						  // reset counter
							keepgoing = false;
						  running_index = 0;
							// FLIP switch
							inswitch = not inswitch;
						  std::cerr << "DADACoadd::SWITCHING" << std::endl;
						  // go back to LOOPING
						  continue;
						}
						else if( read_chunk < sample_chunk ) {
							// fill zeros at the end
							std::fill(data_f + sample_chunk - read_chunk, data_f + sample_chunk, 0.0f);
							vote = true;
							keepgoing = true;
						}
						else if( read_chunk == sample_chunk ) {
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
					}
				}
		public:
				DADACoadd(key_t key_in_1, 
						key_t key_in_2,
						key_t key_out_, 
						bool filout_, 
						timeslice nsamps_,
						int nchans_,
						int nbits_,
						int root_) 
					:
						key_in1(key_in_1), 
						key_in2(key_in_2), 
						key_out(key_out_),
						filout(filout_),
						nsamps(nsamps_),
						nchans(nchans_),
						nbits(nbits_),
						dadain1(key_in1, nsamps, nchans, nbits),
						dadain2(key_in2, nsamps, nchans, nbits),
						inswitch(true),
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
												// dadaout in root
												dadaout = PsrDADA(key_out, nsamps, nchans, nbits);
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
