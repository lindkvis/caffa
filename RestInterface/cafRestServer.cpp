// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
//    Based on example code from Boost Beast:
//    Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
//    Distributed under the Boost Software License, Version 1.0. (See accompanying
//    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//    Official repository: https://github.com/boostorg/beast
//
// ##################################################################################################
#include "cafRestServer.h"

#include "cafFactory.h"
#include "cafLogger.h"
#include "cafRestServiceInterface.h"
#include "cafStringTools.h"

#include <nlohmann/json.hpp>

#include <boost/beast/version.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace caffa::rpc;

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type( beast::string_view path )
{
    return "application/json";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat( beast::string_view base, beast::string_view path )
{
    if ( base.empty() ) return std::string( path );
    std::string result( base );
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if ( result.back() == path_separator ) result.resize( result.size() - 1 );
    result.append( path.data(), path.size() );
    for ( auto& c : result )
        if ( c == '/' ) c = path_separator;
#else
    char constexpr path_separator = '/';
    if ( result.back() == path_separator ) result.resize( result.size() - 1 );
    result.append( path.data(), path.size() );
#endif
    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request( std::shared_ptr<WebSession>                          session,
                     beast::string_view                                   docRoot,
                     http::request<Body, http::basic_fields<Allocator>>&& req,
                     Send&&                                               send )
{
    // Returns an error response
    auto const createResponse = [&req]( http::status status, beast::string_view response )
    {
        http::response<http::string_body> res{ status, req.version() };
        res.set( http::field::server, BOOST_BEAST_VERSION_STRING );
        if ( status == http::status::ok )
        {
            res.set( http::field::content_type, "application/json" );
        }
        else
        {
            res.set( http::field::content_type, "text/plain" );
        }
        res.keep_alive( req.keep_alive() );
        res.body() = std::string( response );
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if ( req.method() != http::verb::post && req.method() != http::verb::delete_ && req.method() != http::verb::patch &&
         req.method() != http::verb::put && req.method() != http::verb::get && req.method() != http::verb::head )
        return send( createResponse( http::status::bad_request, "Unknown HTTP-method" ) );

    // Request path must be absolute and not contain "..".
    if ( req.target().empty() || req.target()[0] != '/' || req.target().find( ".." ) != beast::string_view::npos )
        return send( createResponse( http::status::bad_request, "Illegal request-target" ) );

    std::string target( req.target() );

    CAFFA_DEBUG( "GOT REQUEST: " << target );
    CAFFA_DEBUG( "Request body: " << req.body() );

    std::regex paramRegex( "[\?&]" );

    auto targetComponents = caffa::StringTools::split<std::vector<std::string>>( target, paramRegex );
    if ( targetComponents.empty() )
    {
        return send( createResponse( http::status::bad_request, "Malformed request" ) );
    }

    auto                     path = targetComponents.front();
    std::vector<std::string> params;
    for ( size_t i = 1; i < targetComponents.size(); ++i )
    {
        params.push_back( targetComponents[i] );
        CAFFA_DEBUG( "Found parameter: " << targetComponents[i] );
    }

    std::shared_ptr<caffa::rpc::RestServiceInterface> service;

    auto pathComponents = caffa::StringTools::split<std::list<std::string>>( path, "/", true );

    if ( pathComponents.size() >= 1u )
    {
        auto docOrServiceComponent = pathComponents.front();
        service                    = session->service( docOrServiceComponent );
        if ( service )
        {
            pathComponents.pop_front();
        }
    }

    if ( !service )
    {
        service = session->service( "object" );
    }

    CAFFA_ASSERT( service );

    nlohmann::json jsonArguments = nlohmann::json::object();
    if ( !req.body().empty() )
    {
        try
        {
            jsonArguments = nlohmann::json::parse( req.body() );
        }
        catch ( const nlohmann::detail::parse_error& )
        {
            CAFFA_ERROR( "Could not parse arguments \'" << req.body() << "\'" );
            return send( createResponse( http::status::bad_request,
                                         std::string( "Could not parse arguments \'" ) + req.body() + "\'" ) );
        }
    }
    nlohmann::json jsonMetaData = nlohmann::json::object();
    for ( auto param : params )
    {
        auto keyValue = caffa::StringTools::split<std::vector<std::string>>( param, "=", true );
        if ( keyValue.size() == 2 )
        {
            if ( auto intValue = caffa::StringTools::toInt64( keyValue[1] ); intValue )
            {
                jsonMetaData[keyValue[0]] = *intValue;
            }
            else if ( auto doubleValue = caffa::StringTools::toDouble( keyValue[1] ); doubleValue )
            {
                jsonMetaData[keyValue[0]] = *doubleValue;
            }
            else if ( strcasecmp( "true", keyValue[1].c_str() ) == 0 )
            {
                jsonMetaData[keyValue[0]] = true;
            }
            else if ( strcasecmp( "false", keyValue[1].c_str() ) == 0 )
            {
                jsonMetaData[keyValue[0]] = false;
            }
            else
            {
                jsonMetaData[keyValue[0]] = keyValue[1];
            }
        }
    }

    CAFFA_DEBUG( "Arguments: " << jsonArguments.dump() );
    CAFFA_DEBUG( "Meta data: " << jsonMetaData.dump() );

    try
    {
        auto [status, message] = service->perform( req.method(), pathComponents, jsonArguments, jsonMetaData );
        if ( status == http::status::ok )
        {
            CAFFA_DEBUG( "Responding with " << status << ": " << message );
        }
        else
        {
            CAFFA_ERROR( "Responding with " << status << ": " << message );
        }

        return send( createResponse( status, message ) );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Got exception: " << e.what() );
        return send( createResponse( http::status::internal_server_error, e.what() ) );
    }
}

//------------------------------------------------------------------------------

// Report a failure
void fail( beast::error_code ec, char const* what )
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// This is the C++11 equivalent of a generic lambda.
// The function object is used to send an HTTP message.
struct WebSession::SendLambda
{
    WebSession& self_;

    explicit SendLambda( WebSession& self )
        : self_( self )
    {
    }

    template <bool isRequest, class Body, class Fields>
    void operator()( http::message<isRequest, Body, Fields>&& msg ) const
    {
        // The lifetime of the message has to extend
        // for the duration of the async operation so
        // we use a shared_ptr to manage it.
        auto sp = std::make_shared<http::message<isRequest, Body, Fields>>( std::move( msg ) );

        // Store a type-erased version of the shared
        // pointer in the class to keep it alive.
        self_.m_result = sp;

        // Write the response
        http::async_write( self_.m_stream,
                           *sp,
                           beast::bind_front_handler( &WebSession::onWrite, self_.shared_from_this(), sp->need_eof() ) );
    }
};

WebSession::WebSession( tcp::socket&&                                                       socket,
                        const std::string&                                                  docRoot,
                        const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services )
    : m_stream( std::move( socket ) )
    , m_docRoot( docRoot )
    , m_lambda( std::make_shared<SendLambda>( *this ) )
    , m_services( services )
{
}

// Start the asynchronous operation
void WebSession::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch( m_stream.get_executor(), beast::bind_front_handler( &WebSession::read, shared_from_this() ) );
}

void WebSession::read()
{
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    m_request = {};

    // Set the timeout.
    m_stream.expires_after( std::chrono::seconds( 30 ) );

    // Read a request
    http::async_read( m_stream, m_buffer, m_request, beast::bind_front_handler( &WebSession::onRead, shared_from_this() ) );
}

void WebSession::onRead( beast::error_code ec, std::size_t bytes_transferred )
{
    boost::ignore_unused( bytes_transferred );

    // This means they closed the connection
    if ( ec == http::error::end_of_stream ) return close();

    if ( ec ) return fail( ec, "read" );

    // Send the response
    handle_request( shared_from_this(), m_docRoot, std::move( m_request ), *m_lambda );
}

void WebSession::onWrite( bool closeConnection, beast::error_code ec, std::size_t bytes_transferred )
{
    boost::ignore_unused( bytes_transferred );

    if ( ec ) return fail( ec, "write" );

    if ( closeConnection )
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return close();
    }

    // We're done with the response so delete it
    m_result = nullptr;

    // Read another request
    read();
}

void WebSession::close()
{
    // Send a TCP shutdown
    beast::error_code ec;
    m_stream.socket().shutdown( tcp::socket::shutdown_send, ec );

    // At this point the connection is closed gracefully
}

std::shared_ptr<RestServiceInterface> WebSession::service( const std::string& key ) const
{
    CAFFA_DEBUG( "Trying to find service " << key );

    for ( const auto& [serviceName, service] : m_services )
    {
        CAFFA_DEBUG( "Trying " << serviceName );
        if ( serviceName == key ) return service;
    }
    return nullptr;
}

std::map<std::string, std::shared_ptr<RestServiceInterface>> WebSession::services() const
{
    return m_services;
}

std::string WebSession::peer() const
{
    return m_stream.socket().remote_endpoint().address().to_string();
}

RestServer::RestServer( net::io_context& ioc, tcp::endpoint endpoint, const std::string& docRoot )
    : m_ioContext( ioc )
    , m_acceptor( net::make_strand( ioc ) )
    , m_docRoot( docRoot )
{
    for ( auto key : RestServiceFactory::instance()->allKeys() )
    {
        auto service    = std::shared_ptr<RestServiceInterface>( RestServiceFactory::instance()->create( key ) );
        m_services[key] = service;
    }

    beast::error_code ec;

    // Open the acceptor
    m_acceptor.open( endpoint.protocol(), ec );
    if ( ec )
    {
        fail( ec, "open" );
        return;
    }

    // Allow address reuse
    m_acceptor.set_option( net::socket_base::reuse_address( true ), ec );
    if ( ec )
    {
        fail( ec, "set_option" );
        return;
    }

    // Bind to the server address
    m_acceptor.bind( endpoint, ec );
    if ( ec )
    {
        fail( ec, "bind" );
        return;
    }

    // Start listening for connections
    m_acceptor.listen( net::socket_base::max_listen_connections, ec );
    if ( ec )
    {
        fail( ec, "listen" );
        return;
    }
}

// Start accepting incoming connections
void RestServer::run()
{
    accept();
}

void RestServer::quit()
{
}

void RestServer::accept()
{
    // The new connection gets its own strand
    m_acceptor.async_accept( net::make_strand( m_ioContext ),
                             beast::bind_front_handler( &RestServer::onAccept, shared_from_this() ) );
}

void RestServer::onAccept( beast::error_code ec, tcp::socket socket )
{
    if ( ec )
    {
        fail( ec, "accept" );
    }
    else
    {
        // Create the session and run it
        std::make_shared<WebSession>( std::move( socket ), m_docRoot, m_services )->run();
    }

    // Accept another connection
    accept();
}