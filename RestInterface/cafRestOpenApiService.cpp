// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2024- Kontur AS
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
#include "cafRestOpenApiService.h"

#include "cafSession.h"

#include "cafDefaultObjectFactory.h"
#include "cafDocument.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafRestServerApplication.h"
#include "cafRpcObjectConversion.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa::rpc;

RestServiceInterface::ServiceResponse RestOpenApiService::perform( http::verb             verb,
                                                                   std::list<std::string> path,
                                                                   const nlohmann::json&  queryParams,
                                                                   const nlohmann::json&  body )
{
    CAFFA_DEBUG( "Perfoming OpenAPI request" );
    if ( verb != http::verb::get )
    {
        return std::make_tuple( http::status::bad_request, "Only GET requests are allowed for api queries", nullptr );
    }
    CAFFA_ASSERT( !path.empty() );
    path.pop_front();

    auto currentSchema = getOpenApiV31Schema();

    for ( auto currentPathEntry : path )
    {
        if ( currentSchema.contains( currentPathEntry ) )
        {
            currentSchema = currentSchema[currentPathEntry];
        }
        else
        {
            return std::make_tuple( http::status::not_found, "Entry " + currentPathEntry + " not found", nullptr );
        }
    }

    return std::make_tuple( http::status::ok, currentSchema.dump(), nullptr );
}

bool RestOpenApiService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestOpenApiService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

nlohmann::json RestOpenApiService::getOpenApiV31Schema() const
{
    auto root       = nlohmann::json::object();
    root["openapi"] = "3.1.0";

    auto info           = nlohmann::json::object();
    info["version"]     = RestServerApplication::instance()->appInfo().version_string();
    info["title"]       = RestServerApplication::instance()->appInfo().name;
    info["description"] = RestServerApplication::instance()->appInfo().description;

    auto contact     = nlohmann::json::object();
    contact["email"] = RestServerApplication::instance()->appInfo().contactEmail;
    info["contact"]  = contact;
    root["info"]     = info;

    auto components = nlohmann::json::object();
    auto paths      = nlohmann::json::object();

    for ( auto [key, jsonSchema] : RestServiceInterface::basicServiceSchemas() )
    {
        components[key] = jsonSchema;
    }

    for ( auto key : RestServiceFactory::instance()->allKeys() )
    {
        auto service = std::shared_ptr<RestServiceInterface>( RestServiceFactory::instance()->create( key ) );
        for ( auto [key, jsonObject] : service->serviceComponentEntries() )
        {
            components[key] = jsonObject;
        }
        for ( auto [key, jsonObject] : service->servicePathEntries() )
        {
            paths[key] = jsonObject;
        }
    }

    root["components"] = components;
    root["paths"]      = paths;

    return root;
}

std::map<std::string, nlohmann::json> RestOpenApiService::servicePathEntries() const
{
    return {};
}
std::map<std::string, nlohmann::json> RestOpenApiService::serviceComponentEntries() const
{
    return {};
}