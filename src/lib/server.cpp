#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <deque>
#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace network {

using std::set;
using std::vector;
using std::pair;
using std::string;
using std::deque;
using boost::asio::ip::tcp;

class client {
   public:
    virtual ~client();
    virtual void send( const string& message ) = 0;
};

typedef std::shared_ptr<client> client_ptr;

class session : public client, public std::enable_shared_from_this<session> {
   public:
    session( tcp::socket socket ) : _socket( std::move( socket ) ){};

    void start(){};

    // send message to this session
    void send( const string& message ) {
        (void)message;
    };

   private:
    void read(){};

    void write(){};

    tcp::socket _socket;
};

class Server : public std::enable_shared_from_this<Server> {
   public:
    Server( boost::asio::io_service& io_service, const tcp::endpoint& endpoint )
        : _acceptor( io_service, endpoint ), _socket( io_service ){};
    ~Server(){};

    // start server
    void start() {
        accept();
    };

    // broadcast
    void broadcast( std::function<bool( client )> filter ) {
        (void)filter;
    };

    void on( const string& event,
             std::function<void( const string& msg, const session& session,
                                 const Server& server )>
                 fn ) {
        _el.push_back( make_pair( event, fn ) );
    };

    void apply( const string& event, const session& session,
                const string& msg ) {
        for ( auto e : _el ) {
            if ( e.first == event ) {
                ( e.second )( msg, session, *shared_from_this() );
            }
        }
    };

   private:
    void accept() {
        _acceptor.async_accept(
            _socket, [this]( boost::system::error_code ec ) {
                if ( !ec ) {
                    auto ptr =
                        std::make_shared<session>( std::move( _socket ) );
                    _clients.insert( ptr );
                    ptr->start();
                }

                accept();
            } );
    };

    tcp::acceptor _acceptor;
    tcp::socket   _socket;

    std::set<client_ptr> _clients;

    vector<pair<const string&,
                std::function<void( const string& msg, const session& session,
                                    const Server& server )> > >
        _el;
};
}
