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

#include "cafAppEnum.h"
#include "cafDocument.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafRestObjectService.h"
#include "cafRpcApplication.h"
#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafRpcClientPassByValueObjectFactory.h"
#include "cafSession.h"
#include "cafStringEncoding.h"

#include <nlohmann/json.hpp>

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
#include <utility>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http  = beast::http; // from <boost/beast/http.hpp>
namespace net   = boost::asio; // from <boost/asio.hpp>
using tcp       = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using namespace caffa;
using namespace caffa::rpc;
using namespace std::chrono_literals;

class Connector : public std::enable_shared_from_this<Connector>
{
public:
    explicit Connector( net::io_context& ioc )
        : m_resolver( net::make_strand( ioc ) )
        , m_stream( std::make_shared<beast::tcp_stream>( net::make_strand( ioc ) ) )
    {
    }

    void connect( const std::string& host, const int port )
    {
        // Look up the domain name
        m_resolver.async_resolve( host,
                                  std::to_string( port ),
                                  beast::bind_front_handler( &Connector::onResolve, shared_from_this() ) );
    }

    void onResolve( const beast::error_code ec, const tcp::resolver::results_type& results )
    {
        if ( ec )
        {
            CAFFA_ERROR( "Failed to resolve host" );
            m_connectedStream.set_value( std::make_pair( nullptr, "Failed to resolve host" ) );
            return;
        }

        // Set a timeout on the operation
        m_stream->expires_after( 10s );

        // Make the connection on the IP address we get from a lookup
        m_stream->async_connect( results, beast::bind_front_handler( &Connector::onConnect, shared_from_this() ) );
    }

    void onConnect( const beast::error_code ec, const tcp::resolver::results_type::endpoint_type& )
    {
        if ( ec )
        {
            CAFFA_ERROR( "Failed to connect to host: " << ec );
            m_connectedStream.set_value( std::make_pair( nullptr, "Failed to connect to host " + ec.message() ) );
            return;
        }

        // Set a timeout on the operation
        m_stream->expires_after( 10s );

        boost::asio::socket_base::keep_alive option( true );
        m_stream->socket().set_option( option );

        m_connectedStream.set_value( std::make_pair( m_stream, "Success" ) );
    }

    std::pair<std::shared_ptr<beast::tcp_stream>, std::string> wait() { return m_connectedStream.get_future().get(); }

private:
    tcp::resolver                                                            m_resolver;
    std::shared_ptr<beast::tcp_stream>                                       m_stream;
    std::promise<std::pair<std::shared_ptr<beast::tcp_stream>, std::string>> m_connectedStream;
};

class Request : public std::enable_shared_from_this<Request>
{
public:
    explicit Request( beast::tcp_stream& stream, std::string username, std::string password )
        : m_stream( stream )
        , m_username( std::move( username ) )
        , m_password( std::move( password ) )
    {
    }

    // Start the asynchronous operation
    void run( http::verb verb, const std::string& target, const std::string& body )
    {
        // Set up an HTTP GET request message
        m_req.version( 11 );
        m_req.method( verb );
        m_req.target( target );
        m_req.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
        if ( !m_username.empty() && !m_password.empty() )
        {
            CAFFA_DEBUG( "Setting authorisation header!" );
            m_req.set( http::field::authorization, "Basic " + StringTools::encodeBase64( m_username + ":" + m_password ) );
        }
        if ( !body.empty() )
        {
            CAFFA_TRACE( "Setting request body: " << body );
            m_req.body() = body;
            m_req.prepare_payload();
        }

        // Send the HTTP request to the remote host
        http::async_write( m_stream, m_req, beast::bind_front_handler( &Request::onWrite, shared_from_this() ) );
    }

    void onWrite( beast::error_code ec, std::size_t bytes_transferred )
    {
        boost::ignore_unused( bytes_transferred );

        if ( ec )
        {
            CAFFA_ERROR( "Failed to write to stream" );
            m_result.set_value( std::make_pair( http::status::service_unavailable, "Failed to write to stream" ) );
            m_closeConnection.set_value( true );
            return;
        }

        // Receive the HTTP response
        http::async_read( m_stream, m_buffer, m_res, beast::bind_front_handler( &Request::onRead, shared_from_this() ) );
    }

    void onRead( beast::error_code ec, std::size_t bytes_transferred )
    {
        boost::ignore_unused( bytes_transferred );

        if ( ec == http::error::end_of_stream )
        {
            return;
        }

        if ( ec )
        {
            CAFFA_ERROR( "Failed to read from stream from thread ID " << std::this_thread::get_id() << "-> "
                                                                      << ec.message() );
            m_result.set_value( std::make_pair( http::status::service_unavailable, "Failed to read from stream" ) );
            m_closeConnection.set_value( true );
            return;
        }

        m_result.set_value( std::make_pair( m_res.result(), m_res.body() ) );

        // not_connected happens sometimes so don't bother reporting it.
        if ( ec && ec != beast::errc::not_connected )
        {
            CAFFA_ERROR( "Failed to shut down" );
            m_closeConnection.set_value( true );
            return;
        }
        m_closeConnection.set_value( false );
    }

    std::pair<http::status, std::string> wait() { return m_result.get_future().get(); }
    bool                                 closeConnection() { return m_closeConnection.get_future().get(); }

private:
    beast::tcp_stream&                m_stream;
    beast::flat_buffer                m_buffer; // (Must persist between reads)
    http::request<http::string_body>  m_req;
    http::response<http::string_body> m_res;

    std::promise<std::pair<http::status, std::string>> m_result;
    std::promise<bool>                                 m_closeConnection;

    std::string m_username;
    std::string m_password;
};

std::pair<http::status, std::string> RestClient::performRequest( http::verb         verb,
                                                                 const std::string& hostname,
                                                                 int                port,
                                                                 const std::string& target,
                                                                 const std::string& body,
                                                                 const std::string& username,
                                                                 const std::string& password ) const
{
    if ( !m_stream )
    {
        CAFFA_DEBUG( "Connecting to " << hostname << ": " << port );
        auto connector = std::make_shared<Connector>( *m_ioc );
        connector->connect( hostname, port );
        m_ioc->run();

        auto [stream, message] = connector->wait();
        if ( !stream )
        {
            return std::make_pair( http::status::service_unavailable, message );
        }
        m_stream = stream;
        m_ioc->reset();
    }

    auto request = std::make_shared<Request>( *m_stream, username, password );
    request->run( verb, target, body );
    m_ioc->run();

    auto result = request->wait();

    if ( request->closeConnection() )
    {
        CAFFA_ERROR( "Request failed and stream needs to be closed" );
        m_stream.reset();
    }

    m_ioc->reset();

    return result;
}

std::pair<http::status, std::string> RestClient::performGetRequest( const std::string& hostname,
                                                                    int                port,
                                                                    const std::string& target,
                                                                    const std::string& username,
                                                                    const std::string& password ) const
{
    return performRequest( http::verb::get, hostname, port, target, "", username, password );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClient::RestClient( const std::string& hostname, int port /*= 50000 */ )
    : Client( hostname, port )
    , m_ioc( std::make_shared<net::io_context>() )
{
    // Apply current client to the two client object factories.
    ClientPassByRefObjectFactory::instance()->setClient( this );
    ClientPassByValueObjectFactory::instance()->setClient( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClient::~RestClient()
{
    try
    {
        destroySession();
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to destroy session " << e.what() );
    }
    try
    {
        if ( m_keepAliveThread && m_keepAliveThread->joinable() ) m_keepAliveThread->join();
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to shut down keepalive thread gracefully: " << e.what() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Retrieve Application information
//--------------------------------------------------------------------------------------------------
AppInfo RestClient::appInfo() const
{
    auto [status, body] = performGetRequest( hostname(), port(), "/app/info" );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get Server information: " + body );
    }

    auto jsonContent = nlohmann::json::parse( body );

    return jsonContent.get<AppInfo>();
}

//--------------------------------------------------------------------------------------------------
/// Retrieve a top level document (project)
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> RestClient::document( const std::string& documentId ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    CAFFA_TRACE( "Trying to get document: " << documentId );

    auto [status, body] =
        performGetRequest( hostname(),
                           port(),
                           std::string( "/documents/" ) + documentId + "?skeleton=true&session_uuid=" + m_sessionUuid );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get document " + documentId + ": " + body );
    }
    CAFFA_TRACE( "Got document JSON '" << body << "'" );

    JsonSerializer serializer( ClientPassByRefObjectFactory::instance() );
    serializer.setClient( true );
    serializer.setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON );

    auto document = serializer.createObjectFromString( body );

    return document;
}

//--------------------------------------------------------------------------------------------------
/// Retrieve all top level documents
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<ObjectHandle>> RestClient::documents() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] =
        performGetRequest( hostname(), port(), std::string( "/documents/?skeleton=true&session_uuid=" ) + m_sessionUuid );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get document list: " + body );
    }

    auto jsonArray = nlohmann::json::parse( body );
    if ( !jsonArray.is_array() )
    {
        throw std::runtime_error( "Failed to get documents" );
    }

    JsonSerializer serializer( ClientPassByRefObjectFactory::instance() );
    serializer.setClient( true );
    serializer.setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON );
    std::vector<std::shared_ptr<ObjectHandle>> documents;
    for ( const auto& jsonDocument : jsonArray )
    {
        auto document = serializer.createObjectFromString( jsonDocument.dump() );
        documents.push_back( document );
    }
    return documents;
}

//--------------------------------------------------------------------------------------------------
/// Execute a general non-streaming method.
//--------------------------------------------------------------------------------------------------
std::string RestClient::execute( not_null<const ObjectHandle*> selfObject,
                                 const std::string&            methodName,
                                 const std::string&            jsonArguments ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] =
        performRequest( http::verb::post,
                        hostname(),
                        port(),
                        "/objects/" + selfObject->uuid() + "/methods/" + methodName + "?session_uuid=" + m_sessionUuid,
                        jsonArguments );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( body );
    }

    return body;
}

//--------------------------------------------------------------------------------------------------
// Tell the server to stay alive
//--------------------------------------------------------------------------------------------------
void RestClient::sendKeepAlive()
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/sessions/" ) + m_sessionUuid + "?session_uuid=" + m_sessionUuid,
                                          "" );

    if ( status != http::status::accepted && status != http::status::ok )
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
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    m_keepAliveThread = std::make_unique<std::thread>(
        [this]()
        {
            while ( true )
            {
                {
                    std::scoped_lock<std::mutex> lock( m_sessionMutex );
                    if ( m_sessionUuid.empty() ) break;
                }
                try
                {
                    this->sendKeepAlive();
                    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
                }
                catch ( ... )
                {
                    break;
                }
            }
        } );
    CAFFA_DEBUG( "Thread ID: " << m_keepAliveThread->get_id() );
}

//--------------------------------------------------------------------------------------------------
// Check if the server is ready for sessions
//--------------------------------------------------------------------------------------------------
bool RestClient::isReady( Session::Type type ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    CAFFA_TRACE( "Checking if server is ready for sessions" );

    AppEnum<Session::Type> enumType( type );

    auto [status, body] = performGetRequest( hostname(), port(), std::string( "/sessions/?type=" + enumType.label() ) );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to check for session: " + body );
    }

    CAFFA_TRACE( "Got result: " << body );
    auto jsonObject = nlohmann::json::parse( body );
    if ( !jsonObject.contains( "ready" ) )
    {
        throw std::runtime_error( "Malformed ready reply" );
    }

    bool ready = jsonObject["ready"].get<bool>();
    return ready;
}

//--------------------------------------------------------------------------------------------------
// Create a new session
//--------------------------------------------------------------------------------------------------
void RestClient::doCreateSession( Session::Type type, const std::string& username, const std::string& password )
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    AppEnum<Session::Type> enumType( type );

    CAFFA_TRACE( "Creating session of type " << enumType.label() );

    auto [status, body] =
        performRequest( http::verb::post, hostname(), port(), "/sessions?type=" + enumType.label(), "", username, password );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to create session: " + body );
    }

    CAFFA_TRACE( "Got result: " << body );
    auto jsonObject = nlohmann::json::parse( body );
    if ( !jsonObject.contains( "uuid" ) )
    {
        throw std::runtime_error( "Failed to create session" );
    }

    m_sessionUuid = jsonObject["uuid"].get<std::string>();
    CAFFA_DEBUG( "Created session with UUID: " << m_sessionUuid );
}

//--------------------------------------------------------------------------------------------------
// Check the current session
//--------------------------------------------------------------------------------------------------
Session::Type RestClient::checkSession() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto jsonObject    = nlohmann::json::object();
    jsonObject["uuid"] = m_sessionUuid;

    auto [status, body] = performRequest( http::verb::get,
                                          hostname(),
                                          port(),
                                          std::string( "/sessions/" + m_sessionUuid + "?session_uuid=" ) + m_sessionUuid,
                                          jsonObject.dump() );

    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to check session: " + body );
    }

    CAFFA_TRACE( "Got result: " << body );

    auto jsonResult = nlohmann::json::parse( body );
    CAFFA_ASSERT( jsonResult.contains( "type" ) );
    return AppEnum<Session::Type>( jsonResult["type"].get<std::string>() ).value();
}

//--------------------------------------------------------------------------------------------------
// Change the session type
//--------------------------------------------------------------------------------------------------
void RestClient::changeSession( Session::Type newType )
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/sessions/" + m_sessionUuid + "?session_uuid=" ) +
                                              m_sessionUuid + "&type=" + AppEnum<Session::Type>( newType ).label(),
                                          "" );

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
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );

        if ( m_sessionUuid.empty() ) return;

        CAFFA_DEBUG( "Destroying session " << m_sessionUuid );

        auto [status, body] = performRequest( http::verb::delete_,
                                              hostname(),
                                              port(),
                                              std::string( "/sessions/" + m_sessionUuid + "?session_uuid=" ) + m_sessionUuid,
                                              "" );

        if ( status != http::status::accepted && status != http::status::not_found )
        {
            CAFFA_ERROR( "Failed to destroy session: " << body );
            throw std::runtime_error( "Failed to destroy session: " + body );
        }

        CAFFA_TRACE( "Destroyed session " << m_sessionUuid );
        m_sessionUuid = "";
    }

    if ( m_keepAliveThread )
    {
        m_keepAliveThread->join();
        m_keepAliveThread.reset();
    }
}

//--------------------------------------------------------------------------------------------------
// Get the current session ID
//--------------------------------------------------------------------------------------------------
const std::string& RestClient::sessionUuid() const
{
    return m_sessionUuid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::setJson( const ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value )
{
    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          value.dump() );
    if ( status != http::status::accepted )
    {
        throw std::runtime_error( "Failed to set field value" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
nlohmann::json RestClient::getJson( const ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" +
                                                 fieldName + "?skeleton=true&session_uuid=" + m_sessionUuid );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get field value" );
    }

    return nlohmann::json::parse( body );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> RestClient::getChildObject( const ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" +
                                                 fieldName + "?skeleton=true&session_uuid=" + m_sessionUuid );
    if ( status != http::status::ok )
    {
        throw std::runtime_error( "Failed to get field value" );
    }
    CAFFA_TRACE( "Got body: " << body );

    return JsonSerializer( ClientPassByRefObjectFactory::instance() )
        .setClient( true )
        .setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON )
        .createObjectFromString( body );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<ObjectHandle>> RestClient::getChildObjects( const ObjectHandle* objectHandle,
                                                                        const std::string&  fieldName ) const
{
    std::vector<std::shared_ptr<ObjectHandle>> childObjects;

    auto [status, body] = performGetRequest( hostname(),
                                             port(),
                                             std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" +
                                                 fieldName + "?skeleton=true&session_uuid=" + m_sessionUuid );
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

    JsonSerializer serializer( ClientPassByRefObjectFactory::instance() );
    serializer.setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON );
    serializer.setClient( true );

    for ( const auto& jsonObject : jsonArray )
    {
        childObjects.push_back( JsonSerializer( ClientPassByRefObjectFactory::instance() )
                                    .setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON )
                                    .createObjectFromString( jsonObject.dump() ) );
    }
    return childObjects;
}

//--------------------------------------------------------------------------------------------------
/// TODO: add support for index and replace
//--------------------------------------------------------------------------------------------------
void RestClient::setChildObject( const ObjectHandle* objectHandle, const std::string& fieldName, const ObjectHandle* childObject )
{
    auto [status, body] = performRequest( http::verb::put,
                                          hostname(),
                                          port(),
                                          std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          JsonSerializer().setClient( true ).writeObjectToString( childObject ) );
    if ( status != http::status::accepted )
    {
        throw std::runtime_error( "Failed to set child object with status " +
                                  std::to_string( static_cast<unsigned>( status ) ) + " and body: " + body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::removeChildObject( const ObjectHandle* objectHandle, const std::string& fieldName, size_t index )
{
    auto [status, body] = performRequest( http::verb::delete_,
                                          hostname(),
                                          port(),
                                          std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                              "?index=" + std::to_string( index ) + "&session_uuid=" + m_sessionUuid,
                                          "" );
    if ( status != http::status::accepted )
    {
        throw std::runtime_error( "Failed to remove child object with status " +
                                  std::to_string( static_cast<unsigned>( status ) ) + " and body: " + body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::clearChildObjects( const ObjectHandle* objectHandle, const std::string& fieldName )
{
    auto [status, body] = performRequest( http::verb::delete_,
                                          hostname(),
                                          port(),
                                          std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                              "?session_uuid=" + m_sessionUuid,
                                          "" );
    if ( status != http::status::accepted )
    {
        throw std::runtime_error( "Failed to clear all child objects with status " +
                                  std::to_string( static_cast<unsigned>( status ) ) + " and body: " + body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::insertChildObject( const ObjectHandle* objectHandle,
                                    const std::string&  fieldName,
                                    size_t              index,
                                    const ObjectHandle* childObject )
{
    auto childString    = JsonSerializer().setClient( true ).writeObjectToString( childObject );
    auto [status, body] = performRequest( http::verb::post,
                                          hostname(),
                                          port(),
                                          std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                              "?index=" + std::to_string( index ) + "&session_uuid=" + m_sessionUuid,
                                          childString );
    if ( status != http::status::accepted )
    {
        throw std::runtime_error( "Failed to insert child object at index " + std::to_string( index ) + " with status " +
                                  std::to_string( static_cast<unsigned>( status ) ) + " and body: " + body );
    }
}
