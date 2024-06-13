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
// ##################################################################################################
#include "cafRestServer.h"

#include "cafFactory.h"
#include "cafLogger.h"
#include "cafRestAuthenticator.h"
#include "cafRestServerApplication.h"
#include "cafRestServiceInterface.h"
#include "cafStringEncoding.h"
#include "cafStringTools.h"

#include <nlohmann/json.hpp>

#include "httplib.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace caffa::rpc;

RestServer::RestServer( const std::string& clientHost, int threads, int port, std::shared_ptr<const RestAuthenticator> authenticator )
    : m_clientHost( clientHost )
    , m_port( port )
    , m_threads( threads )
    , m_httpServer( std::make_shared<httplib::Server>() )
    , m_authenticator( authenticator )
{
}

RestServer::~RestServer()
{
}

// Start accepting incoming connections
void RestServer::run()
{
    for ( auto key : RestServiceFactory::instance()->allKeys() )
    {
        auto service    = std::shared_ptr<RestServiceInterface>( RestServiceFactory::instance()->create( key ) );
        m_services[key] = service;
    }

    Handler getHandler =
        std::bind( &RestServer::handleRequest, this, HttpVerb::GET, std::placeholders::_1, std::placeholders::_2 );

    Handler putHandler =
        std::bind( &RestServer::handleRequest, this, HttpVerb::PUT, std::placeholders::_1, std::placeholders::_2 );

    Handler postHandler =
        std::bind( &RestServer::handleRequest, this, HttpVerb::POST, std::placeholders::_1, std::placeholders::_2 );

    Handler deleteHandler =
        std::bind( &RestServer::handleRequest, this, HttpVerb::DELETE_, std::placeholders::_1, std::placeholders::_2 );

    m_httpServer->Get( R"(.*)", getHandler );
    m_httpServer->Put( R"(.*)", putHandler );
    m_httpServer->Post( R"(.*)", postHandler );
    m_httpServer->Delete( R"(.*)", deleteHandler );

    CAFFA_INFO( "Server listening to hosts " << m_clientHost << " on port " << m_port );

    m_httpServer->new_task_queue = [this] { return new httplib::ThreadPool( m_threads ); };
    m_httpServer->listen( m_clientHost, m_port );
}

void RestServer::quit()
{
    m_httpServer->stop();
}

bool RestServer::running() const
{
    return m_httpServer->is_running();
}

void RestServer::waitUntilReady() const
{
    m_httpServer->wait_until_ready();
}

void RestServer::handleRequest( HttpVerb verb, const httplib::Request& request, httplib::Response& response )
{
    auto target = request.matches.str();

    if ( target.empty() || target.front() != '/' || target.find( ".." ) != std::string::npos )
    {
        response.status = httplib::StatusCode::BadRequest_400;
        response.set_content( "Illegal request-target", "text/plain" );
        return;
    }

    auto path = target;

    std::shared_ptr<caffa::rpc::RestServiceInterface> service;

    auto pathComponents = caffa::StringTools::split<std::list<std::string>>( path, "/", true );

    if ( !pathComponents.empty() )
    {
        auto serviceComponent = pathComponents.front();
        auto it               = m_services.find( serviceComponent );
        if ( it != m_services.end() )
        {
            service = it->second;
        }
    }

    if ( !service )
    {
        CAFFA_ERROR( "Could not find service " << path );
        response.status = httplib::StatusCode::NotFound_404;
        response.set_content( "Service not found from path " + path, "text/plain" );
        return;
    }

    CAFFA_ASSERT( service );

    bool requiresAuthentication = service->requiresAuthentication( verb, pathComponents );
    bool requiresValidSession   = service->requiresSession( verb, pathComponents );

    if ( !( requiresAuthentication || requiresValidSession ) && RestServiceInterface::refuseDueToTimeLimiter() )
    {
        response.status = httplib::StatusCode::TooManyRequests_429;
        response.set_content( "Too many unauthenticated requests", "text/plain" );
        return;
    }

    requiresValidSession = requiresValidSession && ServerApplication::instance()->requiresValidSession();

    if ( requiresAuthentication )
    {
        std::string authorisation;
        if ( request.has_header( "Authorization" ) )
        {
            authorisation = request.get_header_value( "Authoriation" );
            authorisation = caffa::StringTools::replace( authorisation, "Basic ", "" );
        }

        if ( !m_authenticator->authenticate( caffa::StringTools::decodeBase64( authorisation ) ) )
        {
            if ( authorisation.empty() )
            {
                response.status = httplib::StatusCode::Unauthorized_401;
                response.set_content( "Need to provide username and password", "text/plain" );
                return;
            }

            response.status = httplib::StatusCode::Forbidden_403;
            response.set_content( "Failed to authenticate", "text/plain" );
            return;
        }
    }

    nlohmann::json queryParamsJson = nlohmann::json::object();
    for ( auto [key, value] : request.params )
    {
        if ( auto intValue = caffa::StringTools::toInt64( value ); intValue )
        {
            queryParamsJson[key] = *intValue;
        }
        else if ( auto doubleValue = caffa::StringTools::toDouble( value ); doubleValue )
        {
            queryParamsJson[key] = *doubleValue;
        }
        else if ( caffa::StringTools::tolower( value ) == "true" )
        {
            queryParamsJson[key] = true;
        }
        else if ( caffa::StringTools::tolower( value ) == "false" )
        {
            queryParamsJson[key] = false;
        }
        else
        {
            queryParamsJson[key] = value;
        }
    }

    if ( requiresValidSession )
    {
        caffa::SessionMaintainer session;
        std::string              session_uuid = "NONE";
        if ( queryParamsJson.contains( "session_uuid" ) )
        {
            session_uuid = queryParamsJson["session_uuid"].get<std::string>();
            auto app     = RestServerApplication::instance();
            session      = app->getExistingSession( session_uuid );
        }

        if ( !session )
        {
            response.status = httplib::StatusCode::Forbidden_403;
            response.set_content( "Session '" + session_uuid + "' is not valid", "text/plain" );
            return;
        }
        else
        {
            if ( session->isExpired() )
            {
                response.status = httplib::StatusCode::Forbidden_403;
                response.set_content( "Session '" + session_uuid + "' has expired", "text/plain" );
                return;
            }
        }
    }

    nlohmann::json bodyJson = nlohmann::json::object();
    if ( !request.body.empty() )
    {
        try
        {
            bodyJson = nlohmann::json::parse( request.body );
        }
        catch ( const nlohmann::detail::parse_error& )
        {
            CAFFA_ERROR( "Could not parse arguments \'" << request.body << "\'" );
            response.status = httplib::StatusCode::BadRequest_400;
            response.set_content( std::string( "Could not parse arguments \'" ) + request.body + "\'", "text/plain" );
            return;
        }
    }

    try
    {
        auto [status, message] = service->perform( verb, pathComponents, queryParamsJson, bodyJson );
        if ( status == httplib::StatusCode::OK_200 || status == httplib::StatusCode::Accepted_202 )
        {
            CAFFA_TRACE( "Responding with " << status << ": " << message );
            response.status = status;
            response.set_content( message, "application/json" );
            return;
        }
        else
        {
            CAFFA_ERROR( "Responding with " << status << ": " << message );
            response.status = status;
            response.set_content( message, "text/plain" );
        }
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Got exception: " << e.what() );
        response.status = httplib::StatusCode::InternalServerError_500;
        response.set_content( e.what(), "text/plain" );
    }
}
