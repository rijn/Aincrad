#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <array>
#include <string>
#include <iostream>

namespace network {

using std::string;
using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint) : _acceptor(io_service, endpoint), _socket(io_service)
    {
        accept();
    };
    ~Server();
private:
    void accept() {};

    tcp::acceptor _acceptor;
    tcp::socket _socket;
};

}
