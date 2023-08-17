// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
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
#include "cafRestClient.h"

#include "cafDefaultObjectFactory.h"
#include "cafDocument.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafRestObjectService.h"
#include "cafRpcApplication.h"
#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafRpcClientPassByValueObjectFactory.h"
#include "cafRpcObjectConversion.h"
#include "cafSession.h"

#include <nlohmann/json.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http  = beast::http; // from <boost/beast/http.hpp>
namespace net   = boost::asio; // from <boost/asio.hpp>
using tcp       = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using namespace caffa::rpc;
using namespace std::chrono_literals;

class Request : public std::enable_shared_from_this<Request>
{
public:
    explicit Request( net::io_context& ioc )
        : m_resolver( net::make_strand( ioc ) )
        , m_stream( net::make_strand( ioc ) )
    {
    }

    // Start the asynchronous operation
    void run( http::verb verb, const std::string& host, int port, const std::string& target, const std::string& body )
    {
        // Set up an HTTP GET request message
        m_req.version( 11 );
        m_req.method( verb );
        m_req.target( target );
        m_req.set( http::field::host, host );
        m_req.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
        if ( !body.empty() )
        {
            CAFFA_TRACE( "Setting request body: " << body );
            m_req.body() = body;
            m_req.prepare_payload();
        }

        // Look up the domain name
        m_resolver.async_resolve( host,
                                  std::to_string( port ),
                                  beast::bind_front_handler( &Request::onResolve, shared_from_this() ) );
    }

    void onResolve( beast::error_code ec, tcp::resolver::results_type results )
    {
        if ( ec )
        {
            CAFFA_ERROR( "Failed to resolve host" );
            m_result.set_value( std::make_pair( http::status::network_connect_timeout_error, "Failed to resolve host" ) );
            return;
        }

        // Set a timeout on the operation
        m_stream.expires_after( 1s );

        // Make the connection on the IP address we get from a lookup
        m_stream.async_connect( results, beast::bind_front_handler( &Request::onConnect, shared_from_this() ) );
    }

    void onConnect( beast::error_code ec, tcp::resolver::results_type::endpoint_type )
    {
        if ( ec )
        {
            CAFFA_ERROR( "Failed to connect to host" );
            m_result.set_value( std::make_pair( http::status::network_connect_timeout_error, "Failed to connect to host" ) );
            return;
        }

        // Set a timeout on the operation
        m_stream.expires_after( 2s );

        // Send the HTTP request to the remote host
        http::async_write( m_stream, m_req, beast::bind_front_handler( &Request::onWrite, shared_from_this() ) );
    }

    void onWrite( beast::error_code ec, std::size_t bytes_transferred )
    {
        boost::ignore_unused( bytes_transferred );

        if ( ec )
        {
            CAFFA_ERROR( "Failed to write to stream" );
            m_result.set_value( std::make_pair( http::status::network_connect_timeout_error, "Failed to write to stream" ) );
            return;
        }

        // Receive the HTTP response
        http::async_read( m_stream, m_buffer, m_res, beast::bind_front_handler( &Request::onRead, shared_from_this() ) );
    }

    void onRead( beast::error_code ec, std::size_t bytes_transferred )
    {
        boost::ignore_unused( bytes_transferred );

        if ( ec )
        {
            CAFFA_ERROR( "Failed to read from stream" );
            m_result.set_value(
                std::make_pair( http::status::network_connect_timeout_error, "Failed to read from stream" ) );
            return;
        }

        m_result.set_value( std::make_pair( m_res.result(), m_res.body() ) );

        // Gracefully close the socket
        m_stream.socket().shutdown( tcp::socket::shutdown_both, ec );

        // not_connected happens sometimes so don't bother reporting it.
        if ( ec && ec != beast::errc::not_connected )
        {
            CAFFA_ERROR( "Failed to shut down" );
            return;
        }

        // If we get here then the connection is closed gracefully
    }

    std::pair<http::status, std::string> wait() { return m_result.get_future().get(); }

private:
    tcp::resolver                     m_resolver;
    beast::tcp_stream                 m_stream;
    beast::flat_buffer                m_buffer; // (Must persist between reads)
    http::request<http::string_body>  m_req;
    http::response<http::string_body> m_res;

    std::promise<std::pair<http::status, std::string>> m_result;
};

std::pair<http::status, std::string>
    performRequest( http::verb verb, const std::string& hostname, int port, const std::string& target, const std::string& body )
{
    // The io_context is required for all I/O
    net::io_context ioc;

    auto request = std::make_shared<Request>( ioc );
    request->run( verb, hostname, port, target, body );
    ioc.run();

    auto result = request->wait();

    return result;
}

std::pair<http::status, std::string> performGetRequest( const std::string& hostname, int port, const std::string& target )
{
    return performRequest( http::verb::get, hostname, port, target, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClient::RestClient( caffa::Session::Type sessionType, const std::string& hostname, int port /*= 50000 */ )
    : Client( hostname, port )
{
    // Apply current client to the two client object factories.
    caffa::rpc::ClientPassByRefObjectFactory::instance()->setClient( this );
    caffa::rpc::ClientPassByValueObjectFactory::instance()->setClient( this );

    if ( sessionType != caffa::Session::Type::INVALID )
    {
        createSession( sessionType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClient::~RestClient()
{
    destroySession();
}

//--------------------------------------------------------------------------------------------------
/// Retrieve Application information
//--------------------------------------------------------------------------------------------------
caffa::AppInfo RestClient::appInfo() const
{
    auto [status, body] = performGetRequest( hostname(), port(), "/app/info" );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get Server information: " + body );
    }

    auto jsonContent = nlohmann::json::parse( body );

    return jsonContent.get<caffa::AppInfo>();
}

//--------------------------------------------------------------------------------------------------
/// Retrieve a top level document (project)
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> RestClient::document( const std::string& documentId ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    CAFFA_TRACE( "Trying to get document: " << documentId );

    auto [status, body] =
        performGetRequest( hostname(),
                           port(),
                           std::string( "/" ) + documentId + "?skeleton=true&session_uuid=" + m_sessionUuid );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get document " + documentId + ": " + body );
    }
    CAFFA_TRACE( "Got document JSON '" << body << "'" );

    caffa::JsonSerializer serializer( caffa::rpc::ClientPassByRefObjectFactory::instance() );
    serializer.setSerializationType( Serializer::SerializationType::DATA_SKELETON );
    auto document = serializer.createObjectFromString( body );
    return document;
}

//--------------------------------------------------------------------------------------------------
/// Retrieve all top level documents
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<caffa::ObjectHandle>> RestClient::documents() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] =
        performGetRequest( hostname(), port(), std::string( "/?skeleton=true&session_uuid=" ) + m_sessionUuid );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get document list: " + body );
    }

    auto jsonArray = nlohmann::json::parse( body );
    if ( !jsonArray.is_array() )
    {
        throw std::runtime_error( "Failed to get documents" );
    }

    caffa::JsonSerializer serializer( caffa::rpc::ClientPassByRefObjectFactory::instance() );
    serializer.setSerializationType( Serializer::SerializationType::DATA_SKELETON );

    std::vector<std::shared_ptr<caffa::ObjectHandle>> documents;
    for ( auto jsonDocument : jsonArray )
    {
        auto document = serializer.createObjectFromString( jsonDocument.dump() );
        documents.push_back( document );
    }
    return documents;
}

//--------------------------------------------------------------------------------------------------
/// Execute a general non-streaming method.
//--------------------------------------------------------------------------------------------------
std::string RestClient::execute( caffa::not_null<const caffa::ObjectHandle*> selfObject,
                                 const std::string&                          methodName,
                                 const std::string&                          jsonArguments ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] =
        performRequest( http::verb::put,
                        hostname(),
                        port(),
                        "/object/uuid/" + selfObject->uuid() + "/" + methodName + "?session_uuid=" + m_sessionUuid,
                        jsonArguments );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }

    return body;
}

//--------------------------------------------------------------------------------------------------
/// Tell the server to stop operation. Returns a simple boolean status where true is ok.
//--------------------------------------------------------------------------------------------------
bool RestClient::stopServer()
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] =
        performRequest( http::verb::delete_, hostname(), port(), std::string( "/app/quit?session_uuid=" ) + m_sessionUuid, "" );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to stop server: " + body );
    }

    CAFFA_TRACE( "Stopped server, which also destroys session" );
    m_sessionUuid = "";

    return true;
}

//--------------------------------------------------------------------------------------------------
// Tell the server to stay alive
//--------------------------------------------------------------------------------------------------
void RestClient::sendKeepAlive()
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto jsonObject            = nlohmann::json::object();
    jsonObject["session_uuid"] = m_sessionUuid;

    auto [status, body] =
        performRequest( http::verb::patch, hostname(), port(), std::string( "/session/keepalive" ), jsonObject.dump() );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to keep server alive: " + body );
    }

    CAFFA_TRACE( "Kept session " << m_sessionUuid << " alive" );
}

//--------------------------------------------------------------------------------------------------
// Start sending keep-alives in a thread until the session is destroyed.
//--------------------------------------------------------------------------------------------------
void RestClient::startKeepAliveThread()
{
    m_keepAliveThread = std::make_unique<std::thread>(
        [this]()
        {
            while ( true )
            {
                try
                {
                    this->sendKeepAlive();
                    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                }
                catch ( ... )
                {
                    break;
                }
            }
        } );
}

//--------------------------------------------------------------------------------------------------
// Create a new session
//--------------------------------------------------------------------------------------------------
void RestClient::createSession( caffa::Session::Type type )
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    CAFFA_TRACE( "Creating session of type " << static_cast<unsigned>( type ) );

    auto arguments    = nlohmann::json::object();
    arguments["type"] = static_cast<unsigned>( type );

    auto [status, body] = performRequest( http::verb::put, hostname(), port(), "/session/create", arguments.dump() );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to create session: " + body );
    }

    CAFFA_TRACE( "Got result: " << body );
    auto jsonObject = nlohmann::json::parse( body );
    if ( !jsonObject.contains( "session_uuid" ) )
    {
        throw std::runtime_error( "Failed to create session" );
    }

    m_sessionUuid = jsonObject["session_uuid"].get<std::string>();
    CAFFA_DEBUG( "Created session with UUID: " << m_sessionUuid );
}

//--------------------------------------------------------------------------------------------------
// Check the current session
//--------------------------------------------------------------------------------------------------
caffa::Session::Type RestClient::checkSession() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto jsonObject            = nlohmann::json::object();
    jsonObject["session_uuid"] = m_sessionUuid;

    auto [status, body] =
        performRequest( http::verb::get, hostname(), port(), std::string( "/session/check" ), jsonObject.dump() );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to check session: " + body );
    }

    CAFFA_TRACE( "Got result: " << body );

    auto jsonResult = nlohmann::json::parse( body );
    CAFFA_ASSERT( jsonResult.contains( "type" ) );
    return static_cast<caffa::Session::Type>( jsonResult["type"].get<unsigned>() );
}

//--------------------------------------------------------------------------------------------------
// Change the session type
//--------------------------------------------------------------------------------------------------
void RestClient::changeSession( caffa::Session::Type newType )
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto jsonObject            = nlohmann::json::object();
    jsonObject["session_uuid"] = m_sessionUuid;
    jsonObject["type"]         = static_cast<unsigned>( newType );

    auto [status, body] =
        performRequest( http::verb::put, hostname(), port(), std::string( "/session/change" ), jsonObject.dump() );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to check session: " + body );
    }
}

//--------------------------------------------------------------------------------------------------
// Tell the server to destroy the session
//--------------------------------------------------------------------------------------------------
void RestClient::destroySession()
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    if ( m_sessionUuid.empty() ) return;

    auto jsonObject            = nlohmann::json::object();
    jsonObject["session_uuid"] = m_sessionUuid;

    auto [status, body] =
        performRequest( http::verb::delete_, hostname(), port(), std::string( "/session/destroy" ), jsonObject.dump() );

    if ( status != http::status::ok && status != http::status::not_found )
    {
        throw std::runtime_error( "Failed to destroy session: " + body );
    }

    CAFFA_TRACE( "Destroyed session " << m_sessionUuid );

    m_sessionUuid = "";
}

//--------------------------------------------------------------------------------------------------
// Get the current session ID
//--------------------------------------------------------------------------------------------------
const std::string& RestClient::sessionUuid() const
{
    return m_sessionUuid;
}

//--------------------------------------------------------------------------------------------------
/// Send a ping to the server
//--------------------------------------------------------------------------------------------------
bool RestClient::ping() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] =
        performGetRequest( hostname(), port(), std::string( "/object/documents?session_uuid=" ) + m_sessionUuid );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to ping server: " + body );
    }

    CAFFA_INFO( "Ping response: " << body );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::setJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value )
{
    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          value.dump() );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to set field value" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
nlohmann::json RestClient::getJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                                 "?skeleton=true&session_uuid=" + m_sessionUuid );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get field value" );
    }

    return nlohmann::json::parse( body );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> RestClient::getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                              const std::string& fieldName ) const
{
    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                                 "?skeleton=true&session_uuid=" + m_sessionUuid );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get field value" );
    }
    CAFFA_TRACE( "Got body: " << body );

    return caffa::JsonSerializer( caffa::rpc::ClientPassByRefObjectFactory::instance() )
        .setSerializationType( Serializer::SerializationType::DATA_SKELETON )
        .createObjectFromString( body );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> RestClient::getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                           const std::string&         fieldName ) const
{
    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                                 "?session_uuid=" + m_sessionUuid );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get field value" );
    }

    auto parsedResult = nlohmann::json::parse( body );
    if ( parsedResult.contains( "value" ) )
    {
        parsedResult = parsedResult["value"];
    }

    return caffa::JsonSerializer( caffa::rpc::ClientPassByValueObjectFactory::instance() )
        .setSerializationType( Serializer::SerializationType::DATA_FULL )
        .createObjectFromString( parsedResult.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::deepCopyChildObjectFrom( const caffa::ObjectHandle* objectHandle,
                                          const std::string&         fieldName,
                                          const caffa::ObjectHandle* childObject )
{
    auto childString    = caffa::JsonSerializer().writeObjectToString( childObject );
    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          childString );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<caffa::ObjectHandle>> RestClient::getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                               const std::string& fieldName ) const
{
    std::vector<std::shared_ptr<caffa::ObjectHandle>> childObjects;

    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                                 "?skeleton=true&session_uuid=" + m_sessionUuid );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get field value" );
    }
    CAFFA_TRACE( "Got body: " << body );

    auto jsonArray = nlohmann::json::parse( body );
    if ( !jsonArray.is_array() )
    {
        throw std::runtime_error( "The return value was not an array" );
    }

    for ( auto jsonObject : jsonArray )
    {
        childObjects.push_back( caffa::JsonSerializer( caffa::rpc::ClientPassByRefObjectFactory::instance() )
                                    .setSerializationType( Serializer::SerializationType::DATA_SKELETON )
                                    .createObjectFromString( jsonObject.dump() ) );
    }
    return childObjects;
}

//--------------------------------------------------------------------------------------------------
/// TODO: add support for index and replace
//--------------------------------------------------------------------------------------------------
void RestClient::setChildObject( const caffa::ObjectHandle* objectHandle,
                                 const std::string&         fieldName,
                                 const caffa::ObjectHandle* childObject )
{
    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          caffa::JsonSerializer().writeObjectToString( childObject ) );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index )
{
    auto [status, body] = performRequest( http::verb::delete_,
                                          hostname(),
                                          port(),
                                          std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName + "[" +
                                              std::to_string( index ) +
                                              "]"
                                              "?session_uuid=" +
                                              m_sessionUuid,
                                          "" );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName )
{
    auto [status, body] = performRequest( http::verb::delete_,
                                          hostname(),
                                          port(),
                                          std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          "" );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::insertChildObject( const caffa::ObjectHandle* objectHandle,
                                    const std::string&         fieldName,
                                    size_t                     index,
                                    const caffa::ObjectHandle* childObject )
{
    auto childString    = caffa::JsonSerializer().writeObjectToString( childObject );
    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/uuid/" ) + objectHandle->uuid() + "/" + fieldName + "[" +
                                              std::to_string( index ) +
                                              "]"
                                              "?session_uuid=" +
                                              m_sessionUuid,
                                          childString );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }
}
