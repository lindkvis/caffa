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
#include "cafRestClient.h"

#include "cafAppEnum.h"
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
#include "cafStringEncoding.h"

#include "httplib.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <memory>
#include <sstream>

using namespace caffa::rpc;
using namespace std::chrono_literals;

void throwOnNoResult( const httplib::Result& result )
{
    if ( !result )
    {
        throw std::runtime_error( "Failed to communicate with server: " + httplib::to_string( result.error() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClient::RestClient( const std::string& hostname, int port /*= 50000 */ )
    : Client( hostname, port )
    , m_ioc( std::make_shared<net::io_context>() )
{
    // Apply current client to the two client object factories.
    caffa::rpc::ClientPassByRefObjectFactory::instance()->setClient( this );
    caffa::rpc::ClientPassByValueObjectFactory::instance()->setClient( this );

    m_httpClient = std::make_shared<httplib::Client>( hostname, port );

    m_httpClient->set_connection_timeout( 5s );
    m_httpClient->set_read_timeout( 5s );
    m_httpClient->set_write_timeout( 5s );
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
        CAFFA_WARNING( "Failed to destroy session " << e.what() );
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
caffa::AppInfo RestClient::appInfo() const
{
    auto result = m_httpClient->Get( "/app/info" );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get Server information: " + result->body );
    }

    auto jsonContent = nlohmann::json::parse( result->body );

    return jsonContent.get<caffa::AppInfo>();
}

//--------------------------------------------------------------------------------------------------
/// Retrieve a top level document (project)
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> RestClient::document( const std::string& documentId ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    CAFFA_TRACE( "Trying to get document: " << documentId );

    auto result =
        m_httpClient->Get( std::string( "/documents/" ) + documentId + "?skeleton=true&session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get document " + documentId + ": " + result->body );
    }
    CAFFA_TRACE( "Got document JSON '" << result->body << "'" );

    caffa::JsonSerializer serializer( caffa::rpc::ClientPassByRefObjectFactory::instance() );
    serializer.setSerializationType( Serializer::SerializationType::DATA_SKELETON );
    auto document = serializer.createObjectFromString( result->body );
    return document;
}

//--------------------------------------------------------------------------------------------------
/// Retrieve all top level documents
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<caffa::ObjectHandle>> RestClient::documents() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto result = m_httpClient->Get( std::string( "/documents/?skeleton=true&session_uuid=" ) + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get document list: " + result->body );
    }

    auto jsonArray = nlohmann::json::parse( result->body );
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

    auto result = m_httpClient->Post( std::string( "/objects/" ) + selfObject->uuid() + "/methods/" + methodName +
                                          "?session_uuid=" + m_sessionUuid,
                                      jsonArguments,
                                      "application/json" );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( result->body );
    }

    return result->body;
}

//--------------------------------------------------------------------------------------------------
// Tell the server to stay alive
//--------------------------------------------------------------------------------------------------
void RestClient::sendKeepAlive()
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto result = m_httpClient->Put( std::string( "/sessions/" ) + m_sessionUuid + "?session_uuid=" + m_sessionUuid );

    if ( !result )
    {
        throw std::runtime_error( "Failed to communicate with server: " + httplib::to_string( result.error() ) );
    }

    if ( result->status != httplib::StatusCode::Accepted_202 && result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to keep server alive: " + result->body );
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
bool RestClient::isReady( caffa::Session::Type type ) const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    CAFFA_TRACE( "Checking if server is ready for sessions" );

    caffa::AppEnum<caffa::Session::Type> enumType( type );

    auto result = m_httpClient->Get( std::string( "/sessions/?type=" + enumType.label() ) );

    if ( !result )
    {
        throw std::runtime_error( "Failed to communicate with server: " + httplib::to_string( result.error() ) );
    }

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to check for session: " + result->body );
    }

    CAFFA_TRACE( "Got result: " << result->body );
    auto jsonObject = nlohmann::json::parse( result->body );
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
void RestClient::createSession( caffa::Session::Type type, const std::string& username, const std::string& password )
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    caffa::AppEnum<caffa::Session::Type> enumType( type );

    CAFFA_INFO( "Creating session of type " << enumType.label() );

    httplib::Headers headers = {};
    if ( !username.empty() && !password.empty() )
    {
        headers = { { "Authorization", "Basic " + caffa::StringTools::encodeBase64( username + ":" + password ) } };
    }

    auto result = m_httpClient->Post( "/sessions?type=" + enumType.label(), headers, "", "" );

    CAFFA_INFO( "Got create result" );
    if ( !result )
    {
        throw std::runtime_error( "Failed to communicate with server: " + httplib::to_string( result.error() ) );
    }

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to create session: " + result->body );
    }

    auto jsonObject = nlohmann::json::parse( result->body );
    if ( !jsonObject.contains( "uuid" ) )
    {
        throw std::runtime_error( "Error creating session. Malformed response (no UUID)." );
    }

    m_sessionUuid = jsonObject["uuid"].get<std::string>();
    CAFFA_DEBUG( "Created session with UUID: " << m_sessionUuid );
}

//--------------------------------------------------------------------------------------------------
// Check the current session
//--------------------------------------------------------------------------------------------------
caffa::Session::Type RestClient::checkSession() const
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto jsonObject    = nlohmann::json::object();
    jsonObject["uuid"] = m_sessionUuid;

    auto result = m_httpClient->Get( std::string( "/sessions/" + m_sessionUuid + "?session_uuid=" ) + m_sessionUuid );

    if ( !result )
    {
        throw std::runtime_error( "Failed to communicate with server: " + httplib::to_string( result.error() ) );
    }

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to check session: " + result->body );
    }

    CAFFA_TRACE( "Got result: " << result->body );

    auto jsonResult = nlohmann::json::parse( result->body );
    CAFFA_ASSERT( jsonResult.contains( "type" ) );
    return caffa::AppEnum<caffa::Session::Type>( jsonResult["type"].get<std::string>() ).value();
}

//--------------------------------------------------------------------------------------------------
// Change the session type
//--------------------------------------------------------------------------------------------------
void RestClient::changeSession( caffa::Session::Type newType )
{
    std::scoped_lock<std::mutex> lock( m_sessionMutex );

    auto result = m_httpClient->Put( std::string( "/sessions/" + m_sessionUuid + "?session_uuid=" ) + m_sessionUuid +
                                         "&type=" + caffa::AppEnum<caffa::Session::Type>( newType ).label(),
                                     "",
                                     "" );

    if ( !result )
    {
        throw std::runtime_error( "Failed to communicate with server: " + httplib::to_string( result.error() ) );
    }

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to change session: " + result->body );
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

        auto result =
            m_httpClient->Delete( std::string( "/sessions/" + m_sessionUuid + "?session_uuid=" ) + m_sessionUuid );

        throwOnNoResult( result );

        if ( result->status != httplib::StatusCode::Accepted_202 && result->status != httplib::StatusCode::NotFound_404 )
        {
            CAFFA_ERROR( "Failed to destroy session: " << result->body );
            throw std::runtime_error( "Failed to destroy session: " + result->body );
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
void RestClient::setJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value )
{
    auto result = m_httpClient->Put( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                         "?session_uuid=" + m_sessionUuid,
                                     value.dump(),
                                     "application/json" );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::Accepted_202 )
    {
        throw std::runtime_error( "Failed to set field value: " + result->body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
nlohmann::json RestClient::getJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    auto result = m_httpClient->Get( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                     "?skeleton=true&session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get field value: " + result->body );
    }

    return nlohmann::json::parse( result->body );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> RestClient::getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                              const std::string& fieldName ) const
{
    auto result = m_httpClient->Get( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                     "?skeleton=true&session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get field value: " + result->body );
    }
    CAFFA_TRACE( "Got body: " << result->body );

    return caffa::JsonSerializer( caffa::rpc::ClientPassByRefObjectFactory::instance() )
        .setSerializationType( Serializer::SerializationType::DATA_SKELETON )
        .createObjectFromString( result->body );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> RestClient::getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                           const std::string&         fieldName ) const
{
    auto result = m_httpClient->Get( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                     "?session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get field value: " + result->body );
    }

    auto parsedResult = nlohmann::json::parse( result->body );
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
    auto childString = caffa::JsonSerializer().writeObjectToString( childObject );
    auto result      = m_httpClient->Put( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                         "?session_uuid=" + m_sessionUuid,
                                     childString,
                                     "application/json" );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::Accepted_202 )
    {
        throw std::runtime_error( "Failed to deep copy object with status " +
                                  std::to_string( static_cast<unsigned>( result->status ) ) +
                                  " and body: " + result->body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<caffa::ObjectHandle>> RestClient::getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                               const std::string& fieldName ) const
{
    std::vector<std::shared_ptr<caffa::ObjectHandle>> childObjects;

    auto result = m_httpClient->Get( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                     "?skeleton=true&session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::OK_200 )
    {
        throw std::runtime_error( "Failed to get field value: " + result->body );
    }
    CAFFA_TRACE( "Got body: " << result->body );

    auto jsonArray = nlohmann::json::parse( result->body );
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
    auto result = m_httpClient->Put( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                         "?session_uuid=" + m_sessionUuid,
                                     caffa::JsonSerializer().writeObjectToString( childObject ),
                                     "application/json" );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::Accepted_202 )
    {
        throw std::runtime_error( "Failed to set child object with status " +
                                  std::to_string( static_cast<unsigned>( result->status ) ) +
                                  " and body: " + result->body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index )
{
    auto result = m_httpClient->Get( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                     "?index=" + std::to_string( index ) + "&session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::Accepted_202 )
    {
        throw std::runtime_error( "Failed to remove child object with status " +
                                  std::to_string( static_cast<unsigned>( result->status ) ) +
                                  " and body: " + result->body );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestClient::clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName )
{
    auto result = m_httpClient->Delete( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                        "?session_uuid=" + m_sessionUuid );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::Accepted_202 )
    {
        throw std::runtime_error( "Failed to clear all child objects with status " +
                                  std::to_string( static_cast<unsigned>( result->status ) ) +
                                  " and body: " + result->body );
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
    auto childString = caffa::JsonSerializer().writeObjectToString( childObject );

    auto result = m_httpClient->Post( std::string( "/objects/" ) + objectHandle->uuid() + "/fields/" + fieldName +
                                          "?index=" + std::to_string( index ) + "&session_uuid=" + m_sessionUuid,
                                      childString,
                                      "application/json" );

    throwOnNoResult( result );

    if ( result->status != httplib::StatusCode::Accepted_202 )
    {
        throw std::runtime_error( "Failed to insert child object at index " + std::to_string( index ) +
                                  " with status " + std::to_string( static_cast<unsigned>( result->status ) ) +
                                  " and body: " + result->body );
    }
}
