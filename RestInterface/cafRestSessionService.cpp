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
#include "cafRestSessionService.h"

#include "cafLogger.h"
#include "cafRestServerApplication.h"
#include "cafSession.h"

#include <nlohmann/json.hpp>

using namespace caffa::rpc;

std::pair<http::status, std::string> RestSessionService::perform( http::verb                    verb,
                                                                  const std::list<std::string>& path,
                                                                  const nlohmann::json&         arguments,
                                                                  const nlohmann::json&         metaData )
{
    auto allCallbacks = callbacks();

    if ( path.empty() )
    {
        auto jsonArray = nlohmann::json::array();
        for ( auto [name, callback] : allCallbacks )
        {
            jsonArray.push_back( name );
        }
        return std::make_pair( http::status::ok, jsonArray.dump() );
    }
    auto name = path.front();

    auto it = allCallbacks.find( name );
    if ( it != allCallbacks.end() )
    {
        return it->second( verb, arguments, metaData );
    }
    return std::make_pair( http::status::not_found, "No such method" );
}

std::map<std::string, RestSessionService::ServiceCallback> RestSessionService::callbacks() const
{
    return { { "ready", &RestSessionService::ready },
             { "check", &RestSessionService::check },
             { "change", &RestSessionService::change },
             { "create", &RestSessionService::create },
             { "keepalive", &RestSessionService::keepalive },
             { "destroy", &RestSessionService::destroy } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestSessionService::ready( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_TRACE( "Received ready for session request with arguments " << arguments );

    try
    {
        caffa::Session::Type type = caffa::Session::Type::REGULAR;
        if ( arguments.contains( "type" ) )
        {
            type = caffa::Session::typeFromUint( arguments["type"].get<unsigned>() );
        }
        else if ( metaData.contains( "type" ) )
        {
            type = caffa::Session::typeFromUint( metaData["type"].get<unsigned>() );
        }
        auto jsonResponse = nlohmann::json::object();

        bool ready = RestServerApplication::instance()->readyForSession( type );

        jsonResponse["ready"]          = ready;
        jsonResponse["other_sessions"] = RestServerApplication::instance()->hasActiveSessions();
        return std::make_pair( http::status::ok, jsonResponse.dump() );
    }
    catch ( ... )
    {
        return std::make_pair( http::status::not_found, "Failed to check for session readiness" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestSessionService::check( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_TRACE( "Got session check request with arguments " << arguments );

    std::string session_uuid = "";
    if ( arguments.contains( "session_uuid" ) )
    {
        session_uuid = arguments["session_uuid"].get<std::string>();
    }
    else if ( metaData.contains( "session_uuid" ) )
    {
        session_uuid = metaData["session_uuid"].get<std::string>();
    }
    auto session = RestServerApplication::instance()->getExistingSession( session_uuid );
    if ( !session )
    {
        return std::make_pair( http::status::unauthorized, "Session '" + session_uuid + "' is not valid" );
    }

    auto jsonResponse            = nlohmann::json::object();
    jsonResponse["session_uuid"] = session->uuid();
    jsonResponse["type"]         = static_cast<unsigned>( session->type() );
    jsonResponse["timeout"]      = session->timeout().count();
    return std::make_pair( http::status::ok, jsonResponse.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestSessionService::change( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_TRACE( "Got session check request with arguments " << arguments );

    std::string session_uuid = "";
    if ( arguments.contains( "session_uuid" ) )
    {
        session_uuid = arguments["session_uuid"].get<std::string>();
    }
    else if ( metaData.contains( "session_uuid" ) )
    {
        session_uuid = metaData["session_uuid"].get<std::string>();
    }
    auto session = RestServerApplication::instance()->getExistingSession( session_uuid );
    if ( !session )
    {
        return std::make_pair( http::status::unauthorized, "Session '" + session_uuid + "' is not valid" );
    }
    if ( !arguments.contains( "type" ) && !metaData.contains( "type" ) )
    {
        return std::make_pair( http::status::bad_request, "No new type provided" );
    }

    caffa::Session::Type type = caffa::Session::Type::REGULAR;
    if ( arguments.contains( "type" ) )
    {
        type = caffa::Session::typeFromUint( arguments["type"].get<unsigned>() );
    }
    else if ( metaData.contains( "type" ) )
    {
        type = caffa::Session::typeFromUint( metaData["type"].get<unsigned>() );
    }

    RestServerApplication::instance()->changeSession( session.get(), type );

    auto jsonResponse            = nlohmann::json::object();
    jsonResponse["session_uuid"] = session->uuid();
    jsonResponse["type"]         = static_cast<unsigned>( session->type() );
    jsonResponse["timeout"]      = session->timeout().count();
    return std::make_pair( http::status::ok, jsonResponse.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestSessionService::create( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_DEBUG( "Received create session request" );

    try
    {
        caffa::Session::Type type = caffa::Session::Type::REGULAR;
        if ( arguments.contains( "type" ) )
        {
            type = caffa::Session::typeFromUint( arguments["type"].get<unsigned>() );
        }
        else if ( metaData.contains( "type" ) )
        {
            type = caffa::Session::typeFromUint( metaData["type"].get<unsigned>() );
        }
        auto session = RestServerApplication::instance()->createSession( type );

        CAFFA_TRACE( "Created session: " << session->uuid() );

        auto jsonResponse            = nlohmann::json::object();
        jsonResponse["session_uuid"] = session->uuid();
        jsonResponse["type"]         = static_cast<unsigned>( session->type() );
        jsonResponse["timeout"]      = session->timeout().count();
        return std::make_pair( http::status::ok, jsonResponse.dump() );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to create session with error: " << e.what() );
        return std::make_pair( http::status::unauthorized, e.what() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestSessionService::keepalive( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_TRACE( "Got session keep-alive request with arguments " << arguments );

    std::string session_uuid = "";
    if ( arguments.contains( "session_uuid" ) )
    {
        session_uuid = arguments["session_uuid"].get<std::string>();
    }
    else if ( metaData.contains( "session_uuid" ) )
    {
        session_uuid = metaData["session_uuid"].get<std::string>();
    }

    auto session = RestServerApplication::instance()->getExistingSession( session_uuid );
    if ( !session )
    {
        return std::make_pair( http::status::unauthorized, "Session '" + session_uuid + "' is not valid" );
    }

    session->updateKeepAlive();

    auto jsonResponse            = nlohmann::json::object();
    jsonResponse["session_uuid"] = session->uuid();
    jsonResponse["type"]         = static_cast<unsigned>( session->type() );
    jsonResponse["timeout"]      = session->timeout().count();
    return std::make_pair( http::status::ok, jsonResponse.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestSessionService::destroy( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_DEBUG( "Got destroy session request with arguments " << arguments );

    std::string session_uuid = "";
    if ( arguments.contains( "session_uuid" ) )
    {
        session_uuid = arguments["session_uuid"].get<std::string>();
    }
    else if ( metaData.contains( "session_uuid" ) )
    {
        session_uuid = metaData["session_uuid"].get<std::string>();
    }
    try
    {
        RestServerApplication::instance()->destroySession( session_uuid );
        return std::make_pair( http::status::ok, "Session successfully destroyed" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Session '" << session_uuid
                                   << "' did not exist. It may already have been destroyed due to lack of keepalive" );
        return std::make_pair( http::status::not_found, "Failed to destroy session. It may already have been destroyed." );
    }
}