#pragma once

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

class _Client {
   public:
    virtual ~_Client(){};

    virtual void send( package_ptr msg ) = 0;
    virtual void on(
        string,
        std::function<void( package_ptr, std::shared_ptr<_Client> )> ) = 0;

    virtual std::string hostname() = 0;
};

typedef std::shared_ptr<_Client> client_ptr;

class Client : public _Client, public std::enable_shared_from_this<Client> {
   public:
    Client( const Client& ) = delete;
    Client& operator=( const Client& ) = delete;
    Client( Client&& ) noexcept        = delete;
    Client& operator=( Client&& ) noexcept = delete;

    ~Client(){};

    Client( boost::asio::io_service& io_service,
            tcp::resolver::iterator  endpoint_iterator )
        : _io_service( io_service ), _socket( io_service ) {
        connect( endpoint_iterator );
    }

    void close() {
        _io_service.post( [this]() { _socket.close(); } );
    }

    void send( package_ptr msg ) {
        _io_service.post( [this, msg]() {
            bool write_in_progress = !send_queue.empty();
            send_queue.push_back( msg );
            if ( !write_in_progress ) {
                write();
            }
        } );
    };

    void on( string event, std::function<void( package_ptr, client_ptr )> fn ) {
        _el.push_back( make_pair( event, fn ) );
    };

    void apply( string event, package_ptr msg ) {
        for ( auto e : _el ) {
            if ( e.first == event ) {
                ( e.second )( msg, shared_from_this() );
            }
        }
    };

    std::string hostname() {
        return _hostname;
    };
    std::string _hostname;

   private:
    void connect( tcp::resolver::iterator endpoint_iterator ) {
        boost::asio::async_connect(
            _socket, endpoint_iterator,
            [this]( boost::system::error_code ec, tcp::resolver::iterator ) {
                if ( !ec ) {
                    read_header();
                    apply( "connect", NULL );
                } else {
                    std::cout << "\nError: " << ec.message() << "\n";
                    exit( 1 );
                }
            } );
    }

    void read_header() {
        auto self( shared_from_this() );
        recv_package = std::make_shared<Package>();
        boost::asio::async_read(
            _socket, boost::asio::buffer(
                         recv_package->data(),
                         Package::size_length + Package::header_length ),
            [this, self]( boost::system::error_code ec,
                          std::size_t /*length*/ ) {
                if ( !ec && recv_package->decrypt() ) {
                    read_body();
                } else {
                    close();
                }
            } );
    }

    void read_body() {
        auto self( shared_from_this() );
        boost::asio::async_read(
            _socket, boost::asio::buffer( recv_package->body(),
                                          recv_package->body_length() ),
            [this, self]( boost::system::error_code ec, std::size_t len ) {
                if ( !ec ) {
                    this->apply( "recv_package", recv_package );
                    read_header();
                } else {
                    apply( "link_lost", NULL );
                    close();
                }
            } );
    };

    void write() {
        send_queue.front()->encrypt();
        boost::asio::async_write(
            _socket, boost::asio::buffer( send_queue.front()->data(),
                                          send_queue.front()->length() ),
            [this]( boost::system::error_code ec, std::size_t ) {
                if ( !ec ) {
                    send_queue.pop_front();
                    if ( !send_queue.empty() ) {
                        write();
                    }
                } else {
                    apply( "link_lost", NULL );
                    close();
                }
            } );
    };

    boost::asio::io_service& _io_service;
    tcp::socket              _socket;

    deque<package_ptr> send_queue;

    vector<pair<string, std::function<void( package_ptr, client_ptr )> > > _el;

    package_ptr recv_package;
};
}
