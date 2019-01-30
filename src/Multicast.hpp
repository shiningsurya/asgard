//#include "asgard.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#define MSGMAXSIZE 8192
namespace asio = boost::asio;

namespace agasio {
		class Multicast;
}
class Multicast {
		private:
				std::array<char, MSGMAXSIZE> payload;
				asio::io_service bsio;
				asio::ip::udp::socket tsock, rsock;
				asio::ip::udp::endpoint l_endp, mc_endp, s_endp;
				asio::ip::address listen_ip;
				asio::ip::address mc_ip;
				unsigned short listen_port, mc_port;
				//
				void rxHandler(boost::system::error_code ec, std::size_t len){
						if(!ec) {
								// for now write to std::cout 
								std::cout.write(payload.data(), len);
								std::cout << std::endl;
						}
				}
				void txHandler(boost::system::error_code ec, std::size_t x) {
						if(ec) {
								std::cerr << "Error in sending multicast\n";
						}
				}
		public:
				Multicast(std::string lip, unsigned short l_port, std::string mcip, unsigned short s_port) : 
						listen_ip(asio::ip::address::from_string(lip)), 
						listen_port(l_port),
						mc_ip(asio::ip::address::from_string(mcip)), 
						mc_port(s_port), 
						l_endp(listen_ip, listen_port),
						mc_endp(mc_ip, mc_port),
						rsock(bsio),
						tsock(bsio, mc_endp.protocol()) {
								// tx socket sends multicast
								// rx socket receives multicast
								rsock.open(l_endp.protocol());
								rsock.set_option(asio::ip::udp::socket::reuse_address(false));
								rsock.bind(l_endp);
								rsock.set_option(asio::ip::multicast::join_group(mc_ip));
				}
				~Multicast() {
						Close();
				}
				void Send(std::string& sendthis) {
						std::copy(sendthis.begin(), sendthis.end(), payload.begin());
						tsock.async_send_to(asio::buffer(sendthis), mc_endp, boost::bind(&Multicast::txHandler, this, asio::placeholders::error, asio::placeholders::bytes_transferred));	
						bsio.run();
				}
				void Receive() {
						rsock.async_receive_from(asio::buffer(payload), s_endp, boost::bind(&Multicast::rxHandler, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
						bsio.run();
				}
				void Close() {
						tsock.close();
						rsock.close();
				}
};
