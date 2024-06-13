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

RestServiceInterface::ServiceResponse RestOpenApiService::perform( HttpVerb               verb,
                                                                   std::list<std::string> path,
                                                                   const nlohmann::json&  queryParams,
                                                                   const nlohmann::json&  body )
{
    CAFFA_DEBUG( "Perfoming OpenAPI request" );
    if ( verb != HttpVerb::GET )
    {
        return std::make_pair( httplib::StatusCode::BadRequest_400, "Only GET requests are allowed for api queries" );
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
            return std::make_pair( httplib::StatusCode::NotFound_404, "Entry " + currentPathEntry + " not found" );
        }
    }

    return std::make_pair( httplib::StatusCode::OK_200, currentSchema.dump() );
}

bool RestOpenApiService::requiresAuthentication( HttpVerb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestOpenApiService::requiresSession( HttpVerb verb, const std::list<std::string>& path ) const
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

    for ( auto [basicKey, jsonSchema] : RestServiceInterface::basicServiceSchemas() )
    {
        components[basicKey] = jsonSchema;
    }

    for ( auto serviceKey : RestServiceFactory::instance()->allKeys() )
    {
        auto service = std::shared_ptr<RestServiceInterface>( RestServiceFactory::instance()->create( serviceKey ) );
        for ( auto [componentKey, jsonObject] : service->serviceComponentEntries() )
        {
            components[componentKey] = jsonObject;
        }
        for ( auto [pathKey, jsonObject] : service->servicePathEntries() )
        {
            paths[pathKey] = jsonObject;
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