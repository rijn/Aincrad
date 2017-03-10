#include <array>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <deque>
#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "package.hpp"

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
    virtual void send( const Package& message ) = 0;
};

typedef std::shared_ptr<client> client_ptr;

class _Server {
   public:
    virtual ~_Server(){};
    virtual void leave( client_ptr ) = 0;
    virtual void apply( const string&  event, client_ptr,
                        const Package* msg ) = 0;
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

    void start() {
        client_s =
            boost::lexical_cast<std::string>( _socket.remote_endpoint() );
        prompt( "connect" );
        read_header();
    };

    // send message to this session
    void send( const Package& package ) {
        bool write_in_progress = !send_queue.empty();
        send_queue.push_back( package );
        if ( !write_in_progress ) {
            write();
        }
    };

   private:
    void prompt( const string& msg ) {
#ifdef DEBUG
        std::cout << "[" << client_s << "] " << msg << std::endl;
#endif
    }

    void read_header() {
        boost::asio::async_read(
            _socket, boost::asio::buffer(
                         recv_package.data(),
                         Package::size_length + Package::header_length ),
            [this]( boost::system::error_code ec, std::size_t /*length*/ ) {
                if ( !ec && recv_package.decrypt() ) {
                    read_body();
                } else {
                    prompt( "leave" );
                    _server->apply( "client_leave", shared_from_this(), NULL );
                    _server->leave( shared_from_this() );
                }
            } );
    }

    void read_body() {
        auto self( shared_from_this() );
        boost::asio::async_read(
            _socket, boost::asio::buffer( recv_package.body(),
                                          recv_package.body_length() ),
            [this, self]( boost::system::error_code ec, std::size_t len ) {
                prompt( "recv " + std::to_string( len ) + " bytes." );
                if ( !ec ) {
                    /*
                     *                    // concat buffer
                     *                    buffer = (char*)realloc( buffer, size
                     * + len );
                     *                    memcpy( buffer + size, _buffer, len );
                     *
                     *                    // decrypt buffer
                     *                    analyze_buffer();
                     */

                    read_header();
                } else {
                    prompt( "leave" );
                    _server->apply( "client_leave", shared_from_this(), NULL );
                    _server->leave( shared_from_this() );
                }
            } );
    };

    void write() {
        auto self( shared_from_this() );
        send_queue.front().encrypt();
        boost::asio::async_write(
            _socket, boost::asio::buffer( send_queue.front().data(),
                                          send_queue.front().length() ),
            [this, self]( boost::system::error_code ec, std::size_t ) {
                if ( !ec ) {
                    send_queue.pop_front();
                    if ( !send_queue.empty() ) {
                        write();
                    }
                } else {
                    prompt( "leave" );
                    _server->apply( "client_leave", shared_from_this(), NULL );
                    _server->leave( shared_from_this() );
                }
            } );
    };

    /*
     *    void analyze_buffer() {
     *        auto   temp  = new Package();
     *        size_t _size = 0;
     *        if ( ( _size = temp->decrypt( buffer, size ) ) == 0 ) {
     *            delete temp;
     *            return;
     *        }
     *
     *        _server->apply( "recv_package", shared_from_this(), temp );
     *        delete temp;
     *
     *        // remove front _size char from buffer
     *        char* new_buffer = (char*)malloc( size - _size );
     *        memcpy( new_buffer, buffer + _size, size - _size );
     *        std::swap( new_buffer, buffer );
     *        delete new_buffer;
     *
     *        analyze_buffer();
     *    };
     */

    tcp::socket _socket;
    server_ptr  _server;

    std::string client_s;

    deque<Package> send_queue;

    /*
     *    char _buffer[4096];
     *
     *    char*  buffer;
     *    size_t size;
     */

    Package recv_package;
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

        std::cout << "Server is listening on "
                  << "8888" << std::endl;
    };

    void broadcast( const Package&                    message,
                    std::function<bool( client_ptr )> filter ) {
        for ( auto it = _clients.begin(); it != _clients.end(); ++it ) {
            if ( filter( *it ) ) ( *it )->send( message );
        }
    };

    // server->broadcast([](client_ptr) { return client_ptr->role == terminal;
    // });

    void on(
        const string& event,
        std::function<void( const Package*, client_ptr, server_ptr )> fn ) {
        _el.push_back( make_pair( event, fn ) );
    };

    void apply( const string& event, client_ptr session, const Package* msg ) {
        for ( auto e : _el ) {
            if ( e.first == event ) {
                ( e.second )( msg, session, shared_from_this() );
            }
        }
    };

    void leave( client_ptr cptr ) {
        auto it = _clients.find( cptr );
        _clients.erase( it );

#ifdef DEBUG
        std::cout << "[server] client = " << _clients.size() << std::endl;
#endif
    }

   private:
    void accept() {
        _acceptor.async_accept(
            _socket, [this]( boost::system::error_code ec ) {
                if ( !ec ) {
                    auto ptr = std::make_shared<session>( std::move( _socket ),
                                                          shared_from_this() );
                    _clients.insert( ptr );
#ifdef DEBUG
                    std::cout << "[server] client = " << _clients.size()
                              << std::endl;
#endif
                    ptr->start();
                }

                accept();
            } );
    };

    tcp::acceptor _acceptor;
    tcp::socket   _socket;

    std::set<client_ptr> _clients;

    vector<pair<const string&, std::function<void( const Package*, client_ptr,
                                                   server_ptr )> > >
        _el;
};
}
