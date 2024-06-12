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
#include "cafRestSession.h"

#include "cafRestServerApplication.h"
#include "cafSession.h"

namespace caffa::rpc
{

std::shared_ptr<RestServiceInterface>
    findRestService( const std::string& key, const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services )
{
    auto it = services.find( key );
    return it != services.end() ? it->second : nullptr;
}

// This is the C++11 equivalent of a generic lambda.
// The function object is used to send an HTTP message.
template <class Derived>
struct SendLambda
{
    RestSession<Derived>& m_self;

    explicit SendLambda( RestSession<Derived>& self )
        : m_self( self )
    {
    }

    template <bool isRequest, class Body, class Fields>
    void operator()( http::message<isRequest, Body, Fields>&& msg, bool forceEof = false ) const
    {
        // The lifetime of the message has to extend
        // for the duration of the async operation so
        // we use a shared_ptr to manage it.
        auto sp = std::make_shared<http::message<isRequest, Body, Fields>>( std::move( msg ) );

        // Store a type-erased version of the shared
        // pointer in the class to keep it alive.
        m_self.m_result = sp;

        // Write the response
        http::async_write( m_self.derived().stream(),
                           *sp,
                           beast::bind_front_handler( &RestSession<Derived>::onWrite,
                                                      m_self.derived().shared_from_this(),
                                                      sp->need_eof() || forceEof ) );
    }
};

template <class Body, class Allocator, class Send>
RestServiceInterface::CleanupCallback
    handleRequest( const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                   std::shared_ptr<const RestAuthenticator>                            authenticator,
                   beast::string_view                                                  docRoot,
                   http::request<Body, http::basic_fields<Allocator>>&&                req,
                   Send&&                                                              send )
{
    // Returns a http response
    auto const createResponse = [&req]( http::status status, beast::string_view response )
    {
        http::response<http::string_body> res{ status, req.version() };
        res.set( http::field::server, BOOST_BEAST_VERSION_STRING );
        if ( status == http::status::ok || status == http::status::accepted )
        {
            res.set( http::field::content_type, "application/json" );
        }
        else
        {
            CAFFA_DEBUG( "Sending failure response " << status << " :" << response );
            res.set( http::field::content_type, "text/plain" );
        }

        if ( status == http::status::unauthorized )
        {
            res.set( http::field::www_authenticate, "Basic realm=\"Restricted Area\"" );
        }
        res.set( http::field::access_control_allow_origin, "http://localhost:8080" );
        res.insert( boost::beast::http::field::access_control_allow_methods, "GET, POST, OPTIONS, PUT, PATCH, DELETE" );
        res.insert( boost::beast::http::field::access_control_allow_headers, "X-Requested-With,content-type" );
        res.keep_alive( req.keep_alive() );
        res.body() = std::string( response );
        res.prepare_payload();
        return res;
    };

    auto method = req.method();

    if ( method == http::verb::delete_ )
    {
        CAFFA_DEBUG( "Got delete request" );
    }

    auto accessControlMethod = req.find( http::field::access_control_request_method );
    if ( method == http::verb::options && accessControlMethod != req.end() )
    {
        CAFFA_DEBUG( "Access control method: " << accessControlMethod->value().data() );
        method = http::string_to_verb(
            std::string( accessControlMethod->value().data(), accessControlMethod->value().size() ) );
    }

    // Make sure we can handle the method
    if ( method != http::verb::post && method != http::verb::delete_ && method != http::verb::patch &&
         method != http::verb::put && method != http::verb::get && method != http::verb::head )
    {
        send( createResponse( http::status::bad_request,
                              "Unknown HTTP-method " + std::string( http::to_string( method ) ) ) );
        return nullptr;
    }

    // Request path must be absolute and not contain "..".
    if ( req.target().empty() || req.target()[0] != '/' || req.target().find( ".." ) != beast::string_view::npos )
    {
        send( createResponse( http::status::bad_request, "Illegal request-target" ) );
        return nullptr;
    }

    std::string target( req.target() );

    if ( method == http::verb::delete_ )
    {
        CAFFA_DEBUG( "Target: " << target << ", body length: " << req.body().length() );
    }
    std::regex paramRegex( "[\?&]" );

    auto targetComponents = caffa::StringTools::split<std::vector<std::string>>( target, paramRegex );
    if ( targetComponents.empty() )
    {
        CAFFA_WARNING( "Sending malformed request" );
        send( createResponse( http::status::bad_request, "Malformed request" ) );
        return nullptr;
    }

    auto                     path = targetComponents.front();
    std::vector<std::string> queryParams;
    for ( size_t i = 1; i < targetComponents.size(); ++i )
    {
        queryParams.push_back( targetComponents[i] );
    }

    std::shared_ptr<caffa::rpc::RestServiceInterface> service;

    auto pathComponents = caffa::StringTools::split<std::list<std::string>>( path, "/", true );

    if ( !pathComponents.empty() )
    {
        auto serviceComponent = pathComponents.front();
        service               = findRestService( serviceComponent, services );
    }

    if ( !service )
    {
        CAFFA_ERROR( "Could not find service " << path );
        send( createResponse( http::status::not_found, "Service not found from path " + path ) );
        return nullptr;
    }

    CAFFA_ASSERT( service );

    if ( method == http::verb::delete_ )
    {
        CAFFA_DEBUG( "Found service" );
    }

    bool requiresAuthentication = service->requiresAuthentication( method, pathComponents );
    bool requiresValidSession   = service->requiresSession( method, pathComponents );

    CAFFA_TRACE( "Requires authentication: " << requiresAuthentication << ", Requires session: " << requiresValidSession );

    if ( !( requiresAuthentication || requiresValidSession ) && RestServiceInterface::refuseDueToTimeLimiter() )
    {
        send( createResponse( http::status::too_many_requests, "Too many unauthenticated requests" ) );
        return nullptr;
    }

    requiresValidSession = requiresValidSession && ServerApplication::instance()->requiresValidSession();

    if ( requiresAuthentication )
    {
        auto authorisation = req[http::field::authorization];
        auto trimmed       = caffa::StringTools::replace( std::string( authorisation ), "Basic ", "" );

        if ( !authenticator->authenticate( caffa::StringTools::decodeBase64( trimmed ) ) )
        {
            if ( authorisation.empty() )
            {
                send( createResponse( http::status::unauthorized, "Need to provide password" ) );
                return nullptr;
            }

            send( createResponse( http::status::forbidden, "Failed to authenticate" ) );
            return nullptr;
        }
    }

    nlohmann::json queryParamsJson = nlohmann::json::object();
    for ( auto param : queryParams )
    {
        auto keyValue = caffa::StringTools::split<std::vector<std::string>>( param, "=", true );
        if ( keyValue.size() == 2 )
        {
            if ( auto intValue = caffa::StringTools::toInt64( keyValue[1] ); intValue )
            {
                queryParamsJson[keyValue[0]] = *intValue;
            }
            else if ( auto doubleValue = caffa::StringTools::toDouble( keyValue[1] ); doubleValue )
            {
                queryParamsJson[keyValue[0]] = *doubleValue;
            }
            else if ( caffa::StringTools::tolower( keyValue[1] ) == "true" )
            {
                queryParamsJson[keyValue[0]] = true;
            }
            else if ( caffa::StringTools::tolower( keyValue[1] ) == "false" )
            {
                queryParamsJson[keyValue[0]] = false;
            }
            else
            {
                queryParamsJson[keyValue[0]] = keyValue[1];
            }
        }
    }

    if ( requiresValidSession )
    {
        caffa::SessionMaintainer session;
        std::string              session_uuid = "NONE";
        if ( queryParamsJson.contains( "session_uuid" ) )
        {
            session_uuid = queryParamsJson["session_uuid"].get<std::string>();
            session      = RestServerApplication::instance()->getExistingSession( session_uuid );
        }

        if ( !session )
        {
            send( createResponse( http::status::forbidden, "Session '" + session_uuid + "' is not valid" ) );
            return nullptr;
        }
        else if ( session->isExpired() )
        {
            send( createResponse( http::status::forbidden, "Session '" + session_uuid + "' is not valid" ) );
            return nullptr;
        }
    }

    nlohmann::json bodyJson = nlohmann::json::object();
    if ( !req.body().empty() )
    {
        try
        {
            bodyJson = nlohmann::json::parse( req.body() );
        }
        catch ( const nlohmann::detail::parse_error& )
        {
            CAFFA_ERROR( "Could not parse arguments \'" << req.body() << "\'" );
            send( createResponse( http::status::bad_request,
                                  std::string( "Could not parse arguments \'" ) + req.body() + "\'" ) );
            return nullptr;
        }
    }

    if ( method == http::verb::delete_ )
    {
        CAFFA_DEBUG( "Path: " << path << ", Query Arguments: " << queryParamsJson.dump()
                              << ", Body: " << bodyJson.dump() << ", Method: " << method );
    }

    try
    {
        auto [status, message, cleanupCallback] = service->perform( method, pathComponents, queryParamsJson, bodyJson );
        if ( status == http::status::ok || status == http::status::accepted )
        {
            CAFFA_TRACE( "Responding with " << status << ": " << message );
            send( createResponse( status, message ) );
        }
        else
        {
            CAFFA_ERROR( "Responding with " << status << ": " << message );
            send( createResponse( status, message ), status == http::status::too_many_requests );
        }

        if ( cleanupCallback )
        {
            CAFFA_DEBUG( "Got a cleanup callback that needs to be handled" );
        }

        return cleanupCallback;
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Got exception: " << e.what() );
        send( createResponse( http::status::internal_server_error, e.what() ) );
        return nullptr;
    }
}

template <class Derived>
RestSession<Derived>::RestSession( beast::flat_buffer                                                  buffer,
                                   const std::string&                                                  docRoot,
                                   const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                                   std::shared_ptr<const RestAuthenticator>                            authenticator )
    : m_docRoot( docRoot )
    , m_lambda( std::make_shared<SendLambda<Derived>>( *this ) )
    , m_services( services )
    , m_authenticator( authenticator )
    , m_buffer( std::move( buffer ) )
{
}

template <class Derived>
void RestSession<Derived>::read()
{
    m_request = {}; // Clear request so reuse doesn't mean that the request grows

    // Set the timeout.
    beast::get_lowest_layer( derived().stream() ).expires_after( std::chrono::seconds( 30 ) );

    // Read a request
    http::async_read( derived().stream(),
                      m_buffer,
                      m_request,
                      beast::bind_front_handler( &RestSession<Derived>::onRead, derived().shared_from_this() ) );
}

template <class Derived>
void RestSession<Derived>::onRead( beast::error_code ec, std::size_t bytes_transferred )
{
    // This means they closed the connection
    if ( ec == http::error::end_of_stream ) return derived().sendEof();

    if ( ec && ec != boost::asio::ssl::error::stream_truncated )
    {
        if ( bytes_transferred > 0u )
        {
            CAFFA_WARNING( "Failed to read socket: " + ec.message() + " after reading " << bytes_transferred << " bytes" );
        }
        return;
    }

    // Send the response
    m_cleanupCallback = handleRequest( m_services, m_authenticator, m_docRoot, std::move( m_request ), *m_lambda );
    if ( m_cleanupCallback )
    {
        CAFFA_DEBUG( "Got a cleanup callback from request handler" );
    }
}

template <class Derived>
void RestSession<Derived>::onWrite( bool close, beast::error_code ec, std::size_t bytes_transferred )
{
    boost::ignore_unused( bytes_transferred );

    if ( ec )
    {
        CAFFA_WARNING( "Failed to write socket: " + ec.message() );
        return;
    }

    if ( close )
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        derived().sendEof();

        // At this point the connection is closed gracefully
        if ( m_cleanupCallback ) m_cleanupCallback();

        return;
    }

    // We're done with the response so delete it
    m_result = nullptr;

    if ( m_cleanupCallback ) m_cleanupCallback();

    // Read another request
    read();
}

template <class Derived>
bool RestSession<Derived>::authenticate( const std::string& authorisationHeader ) const
{
    return m_authenticator->authenticate( authorisationHeader );
}

// Access the derived class, this is part of
// the Curiously Recurring Template Pattern idiom.
template <class Derived>
Derived& RestSession<Derived>::derived()
{
    return static_cast<Derived&>( *this );
}

PlainRestSession::PlainRestSession( tcp::socket&&                                                       socket,
                                    beast::flat_buffer                                                  buffer,
                                    const std::string&                                                  docRoot,
                                    const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                                    std::shared_ptr<const RestAuthenticator>                            authenticator )
    : RestSession<PlainRestSession>( std::move( buffer ), docRoot, services, authenticator )
    , m_stream( std::move( socket ) )
{
}

// Called by the base class
beast::tcp_stream& PlainRestSession::stream()
{
    return m_stream;
}

// Start the asynchronous operation
void PlainRestSession::run()
{
    CAFFA_TRACE( "Starting Plain HTTP session with " << m_stream.socket().remote_endpoint().address().to_string() );
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch( m_stream.get_executor(),
                   beast::bind_front_handler( &RestSession<PlainRestSession>::read, shared_from_this() ) );
}

void PlainRestSession::sendEof()
{
    // Send a TCP shutdown
    beast::error_code ec;
    m_stream.socket().shutdown( tcp::socket::shutdown_send, ec );

    // At this point the connection is closed gracefully
}

SslRestSession::SslRestSession( tcp::socket&&                                                       socket,
                                ssl::context&                                                       ctx,
                                beast::flat_buffer                                                  buffer,
                                const std::string&                                                  docRoot,
                                const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                                std::shared_ptr<const RestAuthenticator>                            authenticator )
    : RestSession<SslRestSession>( std::move( buffer ), docRoot, services, authenticator )
    , m_stream( std::move( socket ), ctx )
{
}

// Called by the base class
beast::ssl_stream<beast::tcp_stream>& SslRestSession::stream()
{
    return m_stream;
}

// Start the asynchronous operation
void SslRestSession::run()
{
    auto self = shared_from_this();

    CAFFA_DEBUG( "Starting HTTPS session" );
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session.
    net::dispatch( m_stream.get_executor(),
                   [self]()
                   {
                       // Set the timeout.
                       beast::get_lowest_layer( self->m_stream ).expires_after( std::chrono::seconds( 30 ) );

                       // Perform the SSL handshake
                       // Note, this is the buffered version of the handshake.
                       self->m_stream.async_handshake( ssl::stream_base::server,
                                                       self->m_buffer.data(),
                                                       beast::bind_front_handler( &SslRestSession::onHandshake, self ) );
                   } );
}

void SslRestSession::onHandshake( beast::error_code ec, std::size_t bytes_used )
{
    if ( ec )
    {
        CAFFA_ERROR( "Failed handshake: " << ec.message() );
        return;
    }

    // Consume the portion of the buffer used by the handshake
    m_buffer.consume( bytes_used );

    read();
}

void SslRestSession::sendEof()
{
    // Set the timeout.
    beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 30 ) );

    // Perform the SSL shutdown
    m_stream.async_shutdown( beast::bind_front_handler( &SslRestSession::onShutdown, shared_from_this() ) );
}

void SslRestSession::onShutdown( beast::error_code ec )
{
    if ( ec )
    {
        CAFFA_ERROR( "Failed shutdown: " << ec.message() );
    }

    // At this point the connection is closed (hopefully) gracefully
}

DetectSession::DetectSession( tcp::socket&&                                                       socket,
                              std::shared_ptr<ssl::context>                                       sslContext,
                              const std::string&                                                  docRoot,
                              const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                              std::shared_ptr<const RestAuthenticator>                            authenticator )
    : m_stream( std::move( socket ) )
    , m_sslContext( sslContext )
    , m_docRoot( docRoot )
    , m_services( services )
    , m_authenticator( authenticator )
{
}

// Launch the detector
void DetectSession::run()
{
    if ( m_sslContext )
    {
        // Set the timeout.
        beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 30 ) );

        // Detect a TLS handshake
        async_detect_ssl( m_stream, m_buffer, beast::bind_front_handler( &DetectSession::onDetect, shared_from_this() ) );
    }
    else
    {
        // Launch plain session
        std::make_shared<PlainRestSession>( m_stream.release_socket(), std::move( m_buffer ), m_docRoot, m_services, m_authenticator )
            ->run();
    }
}

// Launch the detector
void DetectSession::onDetect( beast::error_code ec, bool result )
{
    if ( ec )
    {
        CAFFA_ERROR( "Failed to detect session" );
        return;
    }

    if ( result )
    {
        CAFFA_DEBUG( "Detected that SSL should work" );
        // Launch SSL session
        std::make_shared<SslRestSession>( m_stream.release_socket(),
                                          *m_sslContext,
                                          std::move( m_buffer ),
                                          m_docRoot,
                                          m_services,
                                          m_authenticator )
            ->run();
    }
    else
    {
        // Launch plain session
        std::make_shared<PlainRestSession>( m_stream.release_socket(), std::move( m_buffer ), m_docRoot, m_services, m_authenticator )
            ->run();
    }
}

} // namespace caffa::rpc
