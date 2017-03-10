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

class Client {
    /*
     *: public std::enable_shared_from_this<Client> {
     */
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

    void send( const Package& msg ) {
        _io_service.post( [this, msg]() {
            bool write_in_progress = !send_queue.empty();
            std::cout << "push queue" << std::endl;
            send_queue.push_back( msg );
            if ( !write_in_progress ) {
                write();
            }
        } );
    };

    void on(
        const string& event,
        std::function<void( const Package*, std::shared_ptr<Client> )> fn ) {
        _el.push_back( make_pair( event, fn ) );
    };

    void apply( const string& event, const Package* msg ) {
        for ( auto e : _el ) {
            if ( e.first == event ) {
                /*
                 *( e.second )( msg, shared_from_this() );
                 */
            }
        }
    };

   private:
    void connect( tcp::resolver::iterator endpoint_iterator ) {
        boost::asio::async_connect(
            _socket, endpoint_iterator,
            [this]( boost::system::error_code ec, tcp::resolver::iterator ) {
                std::cout << "connected" << std::endl;
                if ( !ec ) {
                    read();
                }
            } );
    }

    void read() {
        boost::asio::async_read(
            _socket, boost::asio::buffer( _buffer, 4096 ),
            [this]( boost::system::error_code ec, std::size_t len ) {
                (void)len;
                if ( !ec ) {
                    // concat buffer
                    buffer = (char*)realloc( buffer, size + len );
                    memcpy( buffer + size, _buffer, len );

                    // decrypt buffer
                    analyze_buffer();

                    read();
                } else {
                    apply( "link_lost", NULL );
                    close();
                }
            } );
    };

    void write() {
        std::cout << "write" << std::endl;
        std::cout.write( send_queue.front().data(),
                         send_queue.front().length() );
        boost::asio::async_write(
            _socket, boost::asio::buffer( send_queue.front().data(),
                                          send_queue.front().length() ),
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

    void analyze_buffer() {
        auto   temp  = new Package();
        size_t _size = 0;
        if ( ( _size = temp->decrypt( buffer, size ) ) == 0 ) {
            delete temp;
            return;
        }

        apply( "recv_package", temp );
        delete temp;

        // remove front _size char from buffer
        char* new_buffer = (char*)malloc( size - _size );
        memcpy( new_buffer, buffer + _size, size - _size );
        std::swap( new_buffer, buffer );
        delete new_buffer;

        analyze_buffer();
    };

    boost::asio::io_service& _io_service;
    tcp::socket              _socket;

    deque<Package> send_queue;

    char _buffer[4096];

    char*  buffer;
    size_t size;

    vector<
        pair<const string&,
             std::function<void( const Package*, std::shared_ptr<Client> )> > >
        _el;
};
}
