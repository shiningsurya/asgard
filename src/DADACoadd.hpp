#pragma once
#include <asgard.hpp>
// DADA
#include <PsrDADA.hpp>
#include "Group.hpp"
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
				int root;
				mpi::environment env;
				mpi::communicator world;
				mpi::communicator addcomm;
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
				// Buffers
				PtrFloat data_f;
				PtrFloat o_data_f;
				PtrByte data_b;
				PtrByte o_data_b;
				// DADA Objects
				PsrDADA dadain1, dadain2, dadaout;
				// coadder
				void coadder() {
					keepgoing = false;
					// ONLY reads from dadains
					dadain1.ReadLock(true);
					dadain2.ReadLock(true);
					// run indefinitely
					// blocks on inswitch dadain Read Header
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
						if( inswitch ){
							read_chunk = dadain1.ReadData(data_f, data_b);
						}
						else {
							read_chunk = dadain2.ReadData(data_f, data_b);
						}
						// SWITCHING
						if( read_chunk < 0 ) {
							// fill zeros because read failed
							// EOD read fail
							std::fill(data_f, data_f + sample_chunk, 0.0f);
							vote = false;
							// FLIP switch
							inswitch = not inswitch;
						}
						else if( read_chunk < sample_chunk ) {
							// fill zeros at the end
							std::fill(data_f + sample_chunk - read_chunk, data_f + sample_chunk, 0.0f);
							vote = true;
						}
						else if( read_chunk == sample_chunk ) {
							// perfect world case
							vote = true;
						}
						// VOTING
						addcomm = world.split(vote);
						// TODO root resolution
						if(world.rank() == root) {
							root = addcomm.rank();
						}
						// actual MPI coadd call
						mpi::reduce(addcomm, data_f, sample_chunk, o_data_f, std::plus<float>(), root);
						// WRITING
						if(addcomm.rank() == root) {
							// write out Header
							dadaout.WriteHeader();
							// write data
							dadaout.WriteData(nsamps, o_data_f, o_data_b, addcomm.size());
							// Filterbankwrite
							if(filout) {
								// write filterbank 
								fbw.WriteFBdata(outptr, boundcheck, sample_chunk, addcomm.size());
								// incrementing as in serial
								// NB see comments in serial code
								boundcheck += (sample_chunk * nbits / 8);
							}
						}
						// KEEPGOING
						keepgoing = true;
					}
					dadain1.ReadLock(false);
					dadain2.ReadLock(false);
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
						key_in1(key_in_2), 
						key_in2(key_in_2), 
						key_out(key_out_),
						filout(filout_),
						nsamps(nsamps_),
						nchans(nchans_),
						nbits(nbits_),
						dadain1(key_in1, nsamps, nchans, nbits),
						dadain2(key_in2, nsamps, nchans, nbits),
						inswitch(true),
						root(root_) {
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
										if(world.rank() == root) {
												// initialize output buffers
												o_data_b = new unsigned char[bytes_chunk];
												o_data_f = new float[sample_chunk];
												// dadaout in root
												dadaout(key_out, nsamps);
												// FilterbankWriter in root
												if(filout) boundcheck = fbw.Initialize();

										}
						}
				~DADACoadd() {
					delete[] data_b;
					delete[] data_f;
				}
};
