#pragma once
#include "asgard.hpp"
using std::cout;
using std::cerr;
using std::endl;

#include "PsrDADA.hpp"

#include "FilterbankJSON.hpp"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>

#include <boost/circular_buffer.hpp>


//Multicast IPs
// commands
static const char mc_vlitegrp[] = "224.3.29.71";
static int mc_heimdall_stream_port = 20001;
// triggers
static const char mc_trigrp[] = "224.3.29.71";
static int mc_trig_port = 20003;



// should match with defs used in mkerr vlite-fast
enum VF_CMD {
  CMD_START,        // "S"
  CMD_STOP,         // "C"
  CMD_QUIT,         // "Q"
  CMD_EVENT,        // "E"
  CMD_NONE,         // "N"
  CMD_FAKE_START,   // "F"
  CMD_FAKE_STOP     // "G"
};

static VF_CMD mc_cmd_resolve ( const std::string& s ) {

  // if buffer is empty
  if ( s.length() == 0 )
    return CMD_NONE;

  // take first byte
  const char cmd = s.at(0);
  if ( cmd == 'S' || cmd == 's' )
    return CMD_START;
  else if ( cmd == 'C' || cmd == 'c' )
    return CMD_STOP;
  else if ( cmd == 'Q' || cmd == 'q' )
    return CMD_QUIT;
  else if ( cmd == 'E' || cmd == 'e' )
    return CMD_EVENT;
  else if ( cmd == 'F' || cmd == 'f' )
    return CMD_FAKE_START;
  else if ( cmd == 'G' || cmd == 'g' )
    return CMD_FAKE_STOP;
  else
    return CMD_NONE;

}

constexpr static int MC_MAXHOSTNAME = 200;
constexpr static int MC_MAXCONNECTIONS = 5;
constexpr static unsigned int MAX_TRIG = 16;
constexpr static int MC_MAXRECV = MAX_TRIG * sizeof(trigger_t);

// adapted from heimdall multicast socket action
// which was written by me.
class MulticastSocket {
  private:
    int mc_sock;
    struct addrinfo hints;
    struct addrinfo * mcinfo;
    // timeout
    struct timeval tv_timeout;
  public:
    MulticastSocket (int timeout_sec = 1, int timeout_usec = 0 ) :
      mc_sock ( -1 ), mcinfo(nullptr) 
    {
        // setting up timeout
        tv_timeout.tv_sec = timeout_sec;
        tv_timeout.tv_usec = timeout_usec;

        // hints to be used in getaddrinfo call
        memset ( &hints, 0, sizeof ( hints ) );
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_flags    = AI_NUMERICHOST;
    }
    bool is_valid() const { return mc_sock != -1; }
    const int get_fd () const { return mc_sock; }
    ~MulticastSocket () {
      if ( mcinfo != nullptr )
        freeaddrinfo ( mcinfo );
      if ( is_valid() )
        ::close (mc_sock);
    }
    bool Setup ( const char * ip, int port )
    {
      // port (int -> char[6])
      char port_str[6];
      snprintf(port_str, 6, "%d", port);

      // get addr info call
      int gai = getaddrinfo(ip, port_str, &hints, &mcinfo);
      if(gai != 0) 
      {
        std::cout << "Error: getaddrinfo returned=" << gai << " --> " << gai_strerror(gai) << std::endl;
        return false;
      }

      // make socket
      mc_sock = socket ( mcinfo->ai_family, mcinfo->ai_socktype, mcinfo->ai_protocol );
      if ( ! is_valid() )
        return false;

      // timeout
      if ( setsockopt ( mc_sock, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout) ) < 0 )
        return false;

      // reuse port
      gai = 1; // reuse gai
      if ( setsockopt ( mc_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &gai, sizeof ( gai ) ) == -1 )
        return false;

      // bind
      if ( bind ( mc_sock, mcinfo->ai_addr, mcinfo->ai_addrlen ) != 0 )
        return false;

      // add membership to mc
      // ipv6 multicast
      if ( mcinfo->ai_family == AF_INET6 )
      {
        struct ipv6_mreq mreq;

        memcpy ( &mreq.ipv6mr_multiaddr,
            &( (struct sockaddr_in6 *) mcinfo->ai_addr )->sin6_addr,
            sizeof ( struct in6_addr )
            );
        mreq.ipv6mr_interface = 0;
        gai = setsockopt ( mc_sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof ( mreq ) );
      }
      // ipv4 multicast
      else
      {
        struct ip_mreq mreq;

        mreq.imr_interface.s_addr = htonl ( INADDR_ANY );
        inet_aton ( ip, &mreq.imr_multiaddr );
        gai = setsockopt ( mc_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof ( mreq ) );
      }

      return true;
    }

    bool send ( const std::string s ) const
    {
      int status = ::sendto ( mc_sock, s.c_str(), s.size(), MSG_NOSIGNAL,
          mcinfo->ai_addr, mcinfo->ai_addrlen
          );
      if ( status == -1 )
        return false;
      else
        return true;
    }

    std::size_t recv ( PtrByte s ) const
    {

      memset ( s, 0, MC_MAXRECV + 1 );

      auto status= ::recv ( mc_sock, s, MC_MAXRECV, 0 );

      if ( status == -1 )
      {
        std::cout << "status == -1   errno == " << errno << "  in MulticastSocket::recv\n";
        return 0;
      }
      else
      {
        return status;
      }
    }

    void set_non_blocking ( const bool b )
    {
      int opts;

      opts = fcntl ( mc_sock,
          F_GETFL );

      if ( opts < 0 )
        return;

      if ( b )
        opts = ( opts | O_NONBLOCK );
      else
        opts = ( opts & ~O_NONBLOCK );

      fcntl ( mc_sock,
          F_SETFL,opts );

    }

    int select_timeout ( float sleep_secs )
    {
      struct timeval timeout;
      fd_set *rdsp = NULL;
      fd_set readset;

      float whole_seconds = floor (sleep_secs);
      float micro_seconds = sleep_secs - whole_seconds;
      micro_seconds *= 100000;

      timeout.tv_sec = (long int) whole_seconds;
      timeout.tv_usec = (long int) micro_seconds;

      FD_ZERO (&readset);
      FD_SET (mc_sock, &readset);
      rdsp = &readset;

      // select returns the number of file descriptors changed
      // 0 if timeout, -1 if error
      return select (mc_sock+1, rdsp, NULL, NULL, &timeout);
    }
};

// adapted from heimdall multicast socket action
// which was written by me.
class FDSelect {
private:
  fd_set readfds;
  struct timeval tv_timeout;

  std::vector<int> fds;
public:
  FDSelect ( int timeout_sec = 0, int timeout_usec = 500 ) 
  {
    // setting up timeout
    tv_timeout.tv_sec = timeout_sec;
    tv_timeout.tv_usec = timeout_usec;

  }

  void RegisterRead( int fd )
  {
    fds.push_back ( fd );
  }


  void ResetRead()
  {
    // zero out the read set
    FD_ZERO ( &readfds );

    // add all the FDs back
    for ( int i = 0; i < fds.size(); i++) 
      FD_SET ( fds[i], &readfds );
  }

  int Select ( std::vector<int>& ifds )
  {
    ResetRead();
    int rc = 0;

    int fdmax = 0;
    for(int i = 0; i < fds.size(); i++) fdmax = fdmax < fds[i] ? fds[i] : fdmax;

    rc = select ( fdmax + 1, &readfds, NULL, NULL, &tv_timeout );

    // some fd read
    if ( rc != 0 )
    {
      for ( int i = 0; i < fds.size(); i++) 
        if ( FD_ISSET ( fds[i], &readfds ) )
          ifds.push_back ( fds[i] );
    }

    return rc;
  }
  std::size_t SizeRead() { return fds.size(); }
};

class TriggerHook {
  private:
    unsigned char trig_buf[MC_MAXRECV +1]; 
    FilterbankJSON fbson;
    MulticastSocket mc_socket;
    FDSelect fds;
    std::vector<int> ifds;
    // header
    struct Header header;
    // cbuffers
    boost::circular_buffer<double> epoch_cb;
    boost::circular_buffer<char*> buffs_cb;
    boost::circular_buffer<double> tmjd_cb;
    int bstart, bstop;
    // trigger
    std::vector<trigger_t> trigs;
    // dada
    key_t dkey;
    timeslice nsamps;
    int nchans, nbits;
    timeslice buffsz;
    float bufflen;
    unsigned int nbufs;
    timeslice readret;
    unsigned int numobs;
  public:
    TriggerHook (
      key_t key_, unsigned int nbufs_,  
      timeslice nsamps_, int nchans_, int nbits_,
      std::string odir
    ) :
      numobs(0), bstart(-1), bstop(-1),
      dkey(key_), nbufs(nbufs_),
      nsamps(nsamps_), nchans(nchans_), nbits(nbits_),
      fbson (odir),
      mc_socket (2, 0), fds (2, 0)
      {
        // setup socket
        if (
          ! mc_socket.Setup ( mc_trigrp, mc_trig_port )
          ) {
          std::cerr << "Multicast setup failed!" << std::endl;
        }
        // register read
        fds.RegisterRead ( mc_socket.get_fd() );
        // check on buffers
        epoch_cb.set_capacity ( nbufs );
        buffs_cb.set_capacity ( nbufs );
        tmjd_cb.set_capacity ( nbufs );
        // buffsz
        buffsz = nsamps * nchans * nbits / 8;
    }
    // checks for trigger
    bool TriggerCheck (trigger_t& tt) {
      fds.Select ( ifds );
      if ( ifds.size() >= 1 ) {
        PtrByte sbuf;
        mc_socket.recv (sbuf);
        // trigger interception
        auto to = reinterpret_cast<const trigger_t*>(sbuf);
        // copy to tt
        std::memcpy (&tt, to, sizeof(trigger_t));
        // resetting
        ifds.clear();
        return true;
      }
      return false;
    }
    bool TriggerCheck (std::vector<trigger_t>& tt) {
      tt.clear();
      fds.Select ( ifds );
      if ( ifds.size() >= 1 ) {
        std::size_t mcrsz = mc_socket.recv (trig_buf);
        // trigger interception
        int numtrigs = mcrsz / sizeof(trigger_t);
        std::cout << "TriggerHook::TriggerCheck numtrig=" << numtrigs << std::endl;
        trigger_t * mctrig = reinterpret_cast<trigger_t*>(trig_buf);
        std::copy(mctrig, mctrig + numtrigs, std::back_inserter(tt));
        // resetting
        ifds.clear();
        return true;
      }
      return false;
    }
    // action based on trigger
    void PrintTriggerAction (const trigger_t& tt) const noexcept {
      cout.precision(17);
      cout << " t0=" << tt.i0 << " t1=" << tt.i1 << endl; 
      cout.precision(6);
      cout << " peak_time=" << tt.peak_time;
      cout << " sn=" << tt.sn << " dm=" << tt.dm << endl;
    }
    void DiagPrint() {
      cout << " size=" << epoch_cb.size() << endl;
      cout << "\t\ti\ttmjd" << endl;
      cout.precision(17);
      for(int i = 0; i < epoch_cb.size(); i++) {
        cout << "\t\t" << i << "\t" << epoch_cb[i] << endl;
      }
      cout.precision(6);
    }
    void Dumper(const trigger_t& trig) {
      timeslice start, offs; 
      double this_start, this_end;
      fbson.DumpHead (header, trig);
      timeslice tstride = nchans * nbits / 8 / header.tsamp * 1E6;
      timeslice size = trig.i1 >= trig.i0 ? (trig.i1 - trig.i0) * tstride : 0L;
      timeslice fullsize = size;
      for(unsigned int ibuf = bstart; ibuf <= bstop; ibuf++) {
          this_start = epoch_cb[ibuf];
          // logic time
          if ( trig.i0 <= this_start ) start = 0; 
          else start = (trig.i0 - this_start) * tstride;
          offs  = std::min( (buffsz - start), size);
          // sanity-check time
          if ( start < 0 || start >= buffsz ) 
            std::cerr << "TriggerHook::Dumper Invalid start." << std::endl;
          else if ( offs < 0 || offs >= buffsz ) 
            std::cerr << "TriggerHook::Dumper Invalid offs." << std::endl;
          else {
            // call time
            fbson.DumpData ( reinterpret_cast<unsigned char*>(buffs_cb[ibuf]), start, offs );
            size -= offs;
          }
      }
      fbson.WritePayload (fullsize);
    }
    // dada interface
    // MAIN method
    void FollowDADA () {
     while(++numobs) {
         std::cout << "TriggerHook::FollowDADA numobs=" << numobs << std::endl;
         PsrDADA dadain(dkey,nsamps,nchans, nbits, "/home/vlite-master/surya/logs/triggerhook.log");  
         auto bytes_chunk = dadain.GetByteChunkSize();
         dadain.ReadLock(true);
         unsigned int going = 0;
         // read loop
         while (going || dadain.ReadHeader()) {
            if(!going) {
              // first epoch
              dadain.PrintHeader();
              header = dadain.GetHeader();
              bufflen = nsamps * header.tsamp / 1e6;
            }
            readret = dadain.ZeroReadData();
            if (readret == -1) {
               std::cout << "TriggerHook::FollowDADA bad ZeroRead" << std::endl;
               going = 0;
               // eod
            }
            else {
              // push_back current epoch
              epoch_cb.push_back ( 
                header.epoch + 
                (going * bufflen) 
                );
              // push_back data block
              buffs_cb.push_back (
                dadain.GetCurrDataBuff ()
              );
              // push_back going
              tmjd_cb.push_back ( 
                header.tstart + 
                (going * bufflen / 86400.0f) 
                );
              going++;
            }
            if (readret < bytes_chunk) going = 0;
           std::cout << "TriggerHook::FollowDADA going merry=" << going << std::endl;
            // trigger
            if (TriggerCheck(trigs)) {
              for(const auto& trig : trigs) {
                //DiagPrint();
                PrintTriggerAction (trig);
                // iterate over cb to find triggers
                bstart = bstop = -1;
                  for(int ibuf = 1; ibuf < epoch_cb.size(); ibuf++) {
                    if ( trig.i0 <= epoch_cb[ibuf] ) {
                            bstart = ibuf-1;
                            break;
                    }
                  }
                  for(int ibuf = 1; ibuf < epoch_cb.size(); ibuf++) {
                    if ( trig.i1 <= epoch_cb[ibuf] ) {
                            bstop = ibuf-1;
                            break;
                    }
                  }
                  // dumper
                  if (bstart < 0 || bstart >= nbufs) 
                    cerr << "TriggerHook::start unclear." << endl;
                  else if (bstop < 0 || bstop >= nbufs) 
                    if(trig.i1 <= (epoch_cb.back() + bufflen)) bstop = epoch_cb.size()-1;
                    else cerr << "TriggerHook::stop unclear." << endl;
                  else {
                    // valid bstart, bstop
                    cout << "TriggerHook::b bstart=" << bstart;
                    cout << " bstop=" << bstop << endl;
                    // dump logic
                    Dumper(trig);
                  }
              }
            }
            // exit condition
            if (!going) break;
            }
         dadain.ReadLock(false);
        }
      }
};
