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
#include "cafRestServerApplication.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa::rpc;
using namespace caffa;

RestServiceInterface::ServiceResponse RestOpenApiService::perform( const http::verb       verb,
                                                                   std::list<std::string> path,
                                                                   const json::object&    queryParams,
                                                                   const json::value&     body )
{
    CAFFA_DEBUG( "Perfoming OpenAPI request" );
    if ( verb != http::verb::get )
    {
        return std::make_pair( http::status::bad_request, "Only GET requests are allowed for api queries" );
    }
    CAFFA_ASSERT( !path.empty() );
    path.pop_front();

    auto currentSchema = getOpenApiV31Schema();

    for ( const auto& currentPathEntry : path )
    {
        if ( const auto it = currentSchema.find( currentPathEntry ); it != currentSchema.end() )
        {
            currentSchema = it->value().as_object();
        }
        else
        {
            return std::make_pair( http::status::not_found, "Entry " + currentPathEntry + " not found" );
        }
    }

    return std::make_pair( http::status::ok, json::dump( currentSchema ) );
}

bool RestOpenApiService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestOpenApiService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

json::object RestOpenApiService::getOpenApiV31Schema()
{
    auto root       = json::object();
    root["openapi"] = "3.1.0";

    auto info           = json::object();
    info["version"]     = RestServerApplication::instance()->appInfo().version_string();
    info["title"]       = RestServerApplication::instance()->appInfo().name;
    info["description"] = RestServerApplication::instance()->appInfo().description;

    auto contact     = json::object();
    contact["email"] = RestServerApplication::instance()->appInfo().contact_email;
    info["contact"]  = contact;
    root["info"]     = info;

    auto components = json::object();
    auto paths      = json::object();

    for ( const auto& [basicKey, jsonSchema] : RestServiceInterface::basicServiceSchemas() )
    {
        components[basicKey] = jsonSchema;
    }

    for ( const auto& serviceKey : RestServiceFactory::instance()->allKeys() )
    {
        auto service = std::shared_ptr<RestServiceInterface>( RestServiceFactory::instance()->create( serviceKey ) );
        for ( const auto& [componentKey, jsonObject] : service->serviceComponentEntries() )
        {
            components[componentKey] = jsonObject;
        }
        for ( const auto& [pathKey, jsonObject] : service->servicePathEntries() )
        {
            paths[pathKey] = jsonObject;
        }
    }

    root["components"] = components;
    root["paths"]      = paths;

    return root;
}

std::map<std::string, json::object> RestOpenApiService::servicePathEntries() const
{
    return {};
}
std::map<std::string, json::object> RestOpenApiService::serviceComponentEntries() const
{
    return {};
}