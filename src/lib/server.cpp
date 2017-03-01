#include <array>
#include <boost/asio.hpp>
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

#include "package.cpp"

namespace network {

using std::set;
using std::vector;
using std::pair;
using std::string;
using std::deque;
using boost::asio::ip::tcp;
using boost::asio::buffer;

using network::Package;

class client {
   public:
    virtual ~client(){};
    virtual void send( const string& message ) = 0;
};

typedef std::shared_ptr<client> client_ptr;

class _Server {
   public:
    virtual ~_Server(){};
    virtual void leave( client_ptr ) = 0;
    virtual void apply( const string& event, client_ptr,
                        const string& msg ) = 0;
};

typedef std::shared_ptr<_Server> server_ptr;

class session : public client, public std::enable_shared_from_this<session> {
   public:
    session( const session& ) = delete;
    session& operator=( const session& ) = delete;
    session( session&& ) noexcept        = delete;
    session& operator=( session&& ) noexcept = delete;

    session( tcp::socket socket, server_ptr server )
        : _socket( std::move( socket ) ), _server( server ){};
    ~session(){};

    void start(){};

    // send message to this session
    void send( const string& message ) {
        auto self( shared_from_this() );
        // compress data into buffer
        (void)message;
        char* buffer = NULL;
        boost::asio::async_write(
            _socket, boost::asio::buffer( buffer, 0 ),
            [this, self]( boost::system::error_code ec,
                          std::size_t /*length*/ ) {
                if ( !ec ) {
                    _server->apply( "send_finish", shared_from_this(), "" );
                } else {
                    _server->apply( "client_leave", shared_from_this(), "" );
                    _server->leave( shared_from_this() );
                }
            } );
    };

   private:
    void read() {
        char _buffer[4096];
        auto self( shared_from_this() );
        boost::asio::async_read(
            _socket, buffer( _buffer, 4096 ),
            [this, self]( boost::system::error_code ec, std::size_t len ) {
                (void)len;
                if ( !ec ) {
                    // extract info
                    /*
                     *_server->apply();
                     */
                } else {
                    _server->apply( "client_leave", shared_from_this(), "" );
                    _server->leave( shared_from_this() );
                }
            } );
    };

    void write(){};

    tcp::socket _socket;
    server_ptr  _server;
};

class Server : public _Server, public std::enable_shared_from_this<Server> {
   public:
    Server( const Server& ) = delete;
    Server& operator=( const Server& ) = delete;
    Server( Server&& ) noexcept        = delete;
    Server& operator=( Server&& ) noexcept = delete;

    Server( boost::asio::io_service& io_service, const tcp::endpoint& endpoint )
        : _acceptor( io_service, endpoint ), _socket( io_service ){};
    ~Server(){};

    // start server
    void start() {
        accept();

        cout << "Server is listening on "
             << "8888" << endl;
    };

    // broadcast
    void broadcast( std::function<bool( client )> filter ) {
        (void)filter;
    };

    void on( const string& event,
             std::function<void( const string&, client_ptr, server_ptr )> fn ) {
        _el.push_back( make_pair( event, fn ) );
    };

    void apply( const string& event, client_ptr session, const string& msg ) {
        for ( auto e : _el ) {
            if ( e.first == event ) {
                ( e.second )( msg, session, shared_from_this() );
            }
        }
    };

    void leave( client_ptr ) {
    }

   private:
    void accept() {
        _acceptor.async_accept(
            _socket, [this]( boost::system::error_code ec ) {
                if ( !ec ) {
                    auto ptr = std::make_shared<session>( std::move( _socket ),
                                                          shared_from_this() );
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
                std::function<void( const string&, client_ptr, server_ptr )> > >
        _el;
};
}
