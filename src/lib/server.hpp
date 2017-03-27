#pragma once

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

class _session {
   public:
    virtual ~_session(){};
    virtual void send( package_ptr message )  = 0;
    virtual const string get_client_s() const = 0;

    virtual bool operator==( const _session& other ) {
        return true;
    }

    std::string hostname;
};

typedef std::shared_ptr<_session> session_ptr;

class _Server {
   public:
    virtual ~_Server(){};
    virtual void leave( session_ptr ) = 0;
    virtual void on( string,
                     std::function<void( package_ptr, session_ptr,
                                         std::shared_ptr<_Server> )> ){};
    virtual void apply( string event, session_ptr, package_ptr msg ) = 0;
    virtual void broadcast( package_ptr,
                            std::function<bool( session_ptr )> ) = 0;
    virtual void sent_to( package_ptr message, std::string hostname ) = 0;
    virtual std::set<session_ptr>& get_clients() = 0;
};

typedef std::shared_ptr<_Server> server_ptr;

class session : public _session, public std::enable_shared_from_this<session> {
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
    void send( package_ptr package ) {
        bool write_in_progress = !send_queue.empty();
        send_queue.push_back( package );
        if ( !write_in_progress ) {
            write();
        }
    };

    bool operator==( const session& other ) {
        return !( this->client_s.compare( other.get_client_s() ) );
    }

    const string get_client_s() const {
        return client_s;
    }

    std::string hostname;

   private:
    void prompt( const string& msg ) {
#ifdef DEBUG
        std::cout << "[" << client_s << "] " << msg << std::endl;
#endif
    }

    void read_header() {
        recv_package = std::make_shared<Package>();
        boost::asio::async_read(
            _socket,
            boost::asio::buffer( recv_package->data(), Package::header_len() ),
            [this]( boost::system::error_code ec, std::size_t /*length*/ ) {
                if ( !ec && recv_package->decrypt() ) {
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
            _socket, boost::asio::buffer( recv_package->body(),
                                          recv_package->body_length() ),
            [this, self]( boost::system::error_code ec, std::size_t len ) {
                prompt( "recv " + std::to_string( len ) + " bytes." );
                if ( !ec ) {
                    if ( recv_package->is_command() ) {
                        _server->apply( "recv_package", shared_from_this(),
                                        recv_package );
                    } else if ( recv_package->is_file() ) {
                    }
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
        send_queue.front()->encrypt();
        boost::asio::async_write(
            _socket, boost::asio::buffer( send_queue.front()->data(),
                                          send_queue.front()->length() ),
            [this, self]( boost::system::error_code ec, std::size_t len ) {
                prompt( "sent " + std::to_string( len ) + " bytes." );
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

    tcp::socket _socket;
    server_ptr  _server;
    std::string client_s;

    deque<package_ptr> send_queue;

    package_ptr recv_package;
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

    void sent_to( package_ptr message, std::string hostname ) {
        broadcast( message, [&]( session_ptr session ) {
            return session->hostname == hostname;
        } );
    }

    void broadcast( package_ptr                        message,
                    std::function<bool( session_ptr )> filter ) {
        for ( auto it = _clients.begin(); it != _clients.end(); ++it ) {
            if ( filter( *it ) ) ( *it )->send( message );
        }
    };

    void on( string event,
             std::function<void( package_ptr, session_ptr, server_ptr )> fn ) {
        _el.push_back( make_pair( event, fn ) );
    };

    void apply( string event, session_ptr session, package_ptr msg ) {
        for ( auto e : _el ) {
            if ( e.first == event ) {
                ( e.second )( msg, session, shared_from_this() );
            }
        }
    };

    void leave( session_ptr cptr ) {
        auto it = _clients.find( cptr );
        _clients.erase( it );

#ifdef DEBUG
        std::cout << "[server] client = " << _clients.size() << std::endl;
#endif
    }

    virtual std::set<session_ptr>& get_clients() {
        return _clients;
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

    std::set<session_ptr> _clients;

    vector<pair<string,
                std::function<void( package_ptr, session_ptr, server_ptr )> > >
        _el;
};
}
