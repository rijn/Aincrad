#pragma once

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/progress.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#define PACKAGE_HEADER AINCRAD_PACKAGE

namespace network {

/*
 * package construction
 * | HEADER 15 bytes | LENGTH 8 bytes | TYPE 4 bytes | CONTENT |
 */

class Package : public std::enable_shared_from_this<Package> {
   public:
    enum { header_length = 15 };
    enum { size_length = 10 };
    enum { _COMMAND = 1, _SEND_FILE = 2, _RECV_FILE = 3 };
    enum { max_body_length = 1024 };

    static uint8_t header_len() {
        return header_length + size_length + 4;
    }

    Package() : _body_length( 0 ) {
        _data =
            (char*)malloc( header_length + size_length + 4 + max_body_length );
        _type = _COMMAND;
    }

    Package( std::string s ) {
        std::uint32_t s_len = s.length();

        _data        = (char*)malloc( header_length + size_length + 4 + s_len );
        _body_length = s_len;
        _type        = _COMMAND;
        encrypt();
        std::memcpy( body(), s.c_str(), _body_length );
    }

    Package( std::shared_ptr<boost::iostreams::mapped_file_source>& file ) {
        // using static_cast will get a bug
        std::uint32_t s_len = boost::numeric_cast<uint32_t>( file->size() );

        _data        = (char*)malloc( header_length + size_length + 4 + s_len );
        _body_length = s_len;
        _type        = _SEND_FILE;
        encrypt();
        std::memcpy( body(), file->data(), _body_length );
    }

    const char* data() const {
        return _data;
    }

    char* data() {
        return _data;
    }

    bool is_command() {
        return _type == _COMMAND;
    }

    bool is_file() {
        return _type == _SEND_FILE || _type == _RECV_FILE;
    }

    bool is_file() const {
        return _type == _SEND_FILE || _type == _RECV_FILE;
    }

    std::uint32_t length() const {
        return header_length + size_length + 4 + _body_length;
    }

    const char* body() const {
        return _type == _RECV_FILE ? _file->data()
                                   : _data + header_length + size_length + 4;
    }

    char* body() {
        return _type == _RECV_FILE ? _file->data()
                                   : _data + header_length + size_length + 4;
    }

    void try_close_file() {
        if ( _type == _RECV_FILE ) {
            _file->close();
        }
    }

    std::uint32_t body_length() const {
        return _body_length;
    }

    void body_length( std::uint32_t new_length ) {
        _body_length                                       = new_length;
        if ( _body_length > max_body_length ) _body_length = max_body_length;
    }

    bool decrypt() {
        char size[size_length + 1] = "";
        std::strncat( size, _data + header_length, size_length );
        _body_length     = std::strtoull( size, nullptr, 0 );
        char type[4 + 1] = "";
        std::strncat( type, _data + header_length + size_length, 4 );
        _type = std::atoi( type );
        if ( _type == _COMMAND ) {
            _data = (char*)realloc(
                _data, header_length + size_length + 4 + _body_length );
        } else if ( _type == _SEND_FILE ) {
            _type = _RECV_FILE;

            boost::filesystem::path full_path(
                boost::filesystem::initial_path<boost::filesystem::path>() );
            full_path = boost::filesystem::system_complete(
                boost::filesystem::path( "temp" ) );
            if ( !boost::filesystem::exists( full_path ) ) {
                boost::filesystem::create_directory( full_path );
            }
            boost::filesystem::directory_iterator end_iter;
            size_t                                file_count = 0;
            for ( boost::filesystem::directory_iterator dir_itr( full_path );
                  dir_itr != end_iter; ++dir_itr ) {
                if ( boost::filesystem::is_regular_file( dir_itr->status() ) ) {
                    ++file_count;
                    std::cout << dir_itr->path().filename() << "\n";
                }
            }

            std::cout << file_count << std::endl;

            boost::iostreams::mapped_file_params params;
            params.new_file_size = boost::numeric_cast<size_t>( _body_length );
            params.path          = "./temp/" + std::to_string( file_count );
            _file =
                std::make_shared<boost::iostreams::mapped_file_sink>( params );
        }
        return true;
    }

    void encrypt() {
        char header[header_length + size_length + 4 + 1] = "";
        std::sprintf( header, "PACKAGE_HEADER%10u%4d",
                      static_cast<uint32_t>( _body_length ),
                      static_cast<int>( _type ) );
        std::memcpy( _data, header, header_length + size_length + 4 );
    }

   private:
    char*         _data;  //[header_length + size_length + max_body_length];
    std::uint32_t _body_length;
    std::size_t   _type;

    std::shared_ptr<boost::iostreams::mapped_file_sink> _file;
};

typedef std::shared_ptr<Package> package_ptr;
};
