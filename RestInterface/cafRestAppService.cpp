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
#include "cafRestAppService.h"

#include "cafLogger.h"
#include "cafRestServerApplication.h"
#include "cafSession.h"

#include <nlohmann/json.hpp>

using namespace caffa::rpc;

std::pair<http::status, std::string> RestAppService::perform( http::verb                    verb,
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestAppService::info( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
    CAFFA_DEBUG( "Received appInfo request" );
    if ( verb != http::verb::get )
    {
        return std::make_pair( http::status::bad_request, "Only GET makes any sense with app/info" );
    }
    auto app     = RestServerApplication::instance();
    auto appInfo = app->appInfo();

    nlohmann::json json = appInfo;
    return std::make_pair( http::status::ok, json.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestAppService::quit( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
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

    RestServerApplication::instance()->quit();
    return std::make_pair( http::status::ok, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<http::status, std::string>
    RestAppService::ping( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData )
{
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

    return std::make_pair( http::status::ok, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, RestAppService::ServiceCallback> RestAppService::callbacks() const
{
    std::map<std::string, ServiceCallback> callbacks = {
        { "info", &RestAppService::info },
        { "quit", &RestAppService::quit },
        { "ping", &RestAppService::ping },
    };
    return callbacks;
}