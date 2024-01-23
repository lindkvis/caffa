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

RestAppService::ServiceResponse RestAppService::perform( http::verb             verb,
                                                         std::list<std::string> path,
                                                         const nlohmann::json&  queryParams,
                                                         const nlohmann::json&  body )
{
    if ( verb == http::verb::get )
    {
        return info();
    }
    else if ( verb == http::verb::delete_ )
    {
        return quit();
    }
    return std::make_tuple( http::status::bad_request, "Only GET or DELETE makes any sense with app requests", nullptr );
}

bool RestAppService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestAppService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    return verb == http::verb::delete_;
}

std::map<std::string, nlohmann::json> RestAppService::servicePathEntries() const
{
    auto infoRequest = nlohmann::json::object();

    auto getContent                = nlohmann::json::object();
    getContent["application/json"] = { { "schema", { { "$ref", "#/components/app_schemas/AppInfo" } } } };
    auto getResponses              = nlohmann::json::object();

    auto errorContent          = nlohmann::json::object();
    errorContent["text/plain"] = { { "schema", { { "$ref", "#/components/error_schemas/PlainError" } } } };

    getResponses[std::to_string( static_cast<unsigned>( http::status::ok ) )] = { { "description",
                                                                                    "Application Information" },
                                                                                  { "content", getContent } };
    getResponses[std::to_string( static_cast<unsigned>( http::status::too_many_requests ) )] =
        { { "description", "Too many Requests Error Message" }, { "content", errorContent } };

    infoRequest["get"] = { { "summary", "Get Application Information" },
                           { "operationId", "info" },
                           { "responses", getResponses },
                           { "tags", { "app" } } };

    auto quitRequest   = nlohmann::json::object();
    auto quitResponses = nlohmann::json::object();

    quitResponses[std::to_string( static_cast<unsigned>( http::status::accepted ) )] = {
        { "description", "Success" },
    };

    quitResponses[std::to_string( static_cast<unsigned>( http::status::forbidden ) )] = { { "description",
                                                                                            "Quit Error Message" },
                                                                                          { "content", errorContent } };

    quitRequest["delete"] = { { "summary", "Quit Application" },
                              { "operationId", "quit" },
                              { "responses", quitResponses },
                              { "tags", { "app" } } };

    return { { "/app/info", infoRequest }, { "/app/quit", quitRequest } };
}

std::map<std::string, nlohmann::json> RestAppService::serviceComponentEntries() const
{
    auto appInfo = AppInfo::jsonSchema();

    return { { "app_schemas", { { "AppInfo", appInfo } } } };
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestAppService::ServiceResponse RestAppService::info()
{
    CAFFA_TRACE( "Received info request" );

    auto app     = RestServerApplication::instance();
    auto appInfo = app->appInfo();

    nlohmann::json json = appInfo;
    return std::make_tuple( http::status::ok, json.dump(), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestAppService::ServiceResponse RestAppService::quit()
{
    CAFFA_DEBUG( "Received quit request" );

    return std::make_tuple( http::status::accepted,
                            "Told to quit. It will happen soon",
                            []() { RestServerApplication::instance()->quit(); } );
}
