#pragma once
#include "asgard.hpp"
#include "Header.hpp"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>


class MulticastSend {
  using Byte = char;
  private:
    int mc_sock;
    char mc_ip[256];
    int mc_port;
    struct sockaddr_in adr;
  public:
    MulticastSend ( const char * ip, const int port) :
      mc_sock ( -1 ), mc_port (port)
    {
        strcpy (mc_ip, ip); 
        mc_sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP); 

        memset(&adr, 0, sizeof(adr));
        adr.sin_family      = AF_INET;
        adr.sin_addr.s_addr = inet_addr(mc_ip);
        adr.sin_port        = htons (mc_port);
    }
    ~MulticastSend () {
      if (mc_sock != -1) close (mc_sock);
    }
    bool is_valid() const { return mc_sock != -1; }
    const int get_fd () const { return mc_sock; }

    bool send ( Byte * s , unsigned size ) 
    {
      int status = ::sendto ( mc_sock, s, size , 0,
        reinterpret_cast<struct sockaddr*>(&adr), sizeof(adr));
      if (status != size) {
        std::cout << "MulticastSend::send sent=" << status << " while had to send=" << size << std::endl;
        return false;
      }
      else return true;
    }

    bool send ( trigger_t& tt ) {
      Byte* pt   = reinterpret_cast<Byte*>(&tt);
      auto  size = sizeof (tt);
      return send (pt, size);
    }

};
