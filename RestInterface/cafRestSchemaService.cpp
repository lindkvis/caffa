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
#include "cafRestSchemaService.h"

#include "cafSession.h"

#include "cafDefaultObjectFactory.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafRestServerApplication.h"
#include "cafRpcObjectConversion.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa::rpc;
using namespace std::chrono_literals;

constexpr std::chrono::seconds RATE_LIMITER_TIME_PERIOD  = 1s;
constexpr size_t               RATE_LIMITER_MAX_REQUESTS = 5;

std::mutex                                       RestSchemaService::s_requestMutex;
std::list<std::chrono::steady_clock::time_point> RestSchemaService::s_requestTimes;

bool RestSchemaService::refuseDueToTimeLimiter()
{
    std::scoped_lock lock( s_requestMutex );

    auto now = std::chrono::steady_clock::now();

    std::list<std::chrono::steady_clock::time_point> recentRequests;
    for ( auto requestTime : s_requestTimes )
    {
        if ( now - requestTime < RATE_LIMITER_TIME_PERIOD )
        {
            recentRequests.push_back( requestTime );
        }
    }

    s_requestTimes.swap( recentRequests );

    if ( s_requestTimes.size() >= RATE_LIMITER_MAX_REQUESTS )
    {
        return true;
    }

    s_requestTimes.push_back( now );
    return false;
}

RestSchemaService::ServiceResponse RestSchemaService::perform( http::verb                    verb,
                                                               const std::list<std::string>& path,
                                                               const nlohmann::json&         arguments,
                                                               const nlohmann::json&         metaData )
{
    if ( verb != http::verb::get )
    {
        return std::make_tuple( http::status::bad_request, "Only GET requests are allowed for schema queries", nullptr );
    }

    std::string session_uuid = "";
    if ( arguments.contains( "session_uuid" ) )
    {
        session_uuid = arguments["session_uuid"].get<std::string>();
    }
    else if ( metaData.contains( "session_uuid" ) )
    {
        session_uuid = metaData["session_uuid"].get<std::string>();
    }

    if ( session_uuid.empty() )
    {
        CAFFA_WARNING( "No session uuid provided" );
    }
    auto session = RestServerApplication::instance()->getExistingSession( session_uuid );

    if ( !session && RestServerApplication::instance()->requiresValidSession() )
    {
        if ( refuseDueToTimeLimiter() )
        {
            return std::make_tuple( http::status::too_many_requests, "Too many unauthenticated schema requests", nullptr );
        }
    }

    if ( path.empty() )
    {
        return getAllSchemas();
    }

    auto factory = DefaultObjectFactory::instance();
    auto object  = factory->create( path.front() );

    if ( !object )
    {
        return std::make_tuple( http::status::not_found, "No such class", nullptr );
    }

    auto reducedPath = path;
    reducedPath.pop_front();
    if ( reducedPath.empty() )
    {
        return std::make_tuple( http::status::ok, createJsonSchemaFromProjectObject( object.get() ).dump(), nullptr );
    }
    return getFieldSchema( object.get(), reducedPath.front() );
}

bool RestSchemaService::requiresAuthentication( const std::list<std::string>& path ) const
{
    return false;
}

RestSchemaService::ServiceResponse RestSchemaService::getAllSchemas()
{
    auto factory = DefaultObjectFactory::instance();

    auto root    = nlohmann::json::object();
    root["$id"]  = "/schemas";
    root["type"] = "object";

    auto oneOf   = nlohmann::json::array();
    auto schemas = nlohmann::json::object();

    for ( auto className : factory->classes() )
    {
        auto object = factory->create( className );

        oneOf.push_back( { { "$ref", "#/schemas/" + className } } );
        schemas[className] = createJsonSchemaFromProjectObject( object.get() );
    }
    root["oneOf"]   = oneOf;
    root["schemas"] = schemas;

    return std::make_tuple( http::status::ok, root.dump(), nullptr );
}

RestSchemaService::ServiceResponse RestSchemaService::getFieldSchema( const caffa::ObjectHandle* object,
                                                                      const std::string&         fieldName )
{
    auto field = object->findField( fieldName );
    if ( !field ) return std::make_tuple( http::status::not_found, "Field does not exist", nullptr );

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field is not remote writable", nullptr );

    auto ioCapability = field->capability<caffa::FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }

    caffa::JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
    serializer.setSerializationType( Serializer::SerializationType::SCHEMA );

    nlohmann::json json;
    ioCapability->writeToJson( json, serializer );
    return std::make_tuple( http::status::ok, json.dump(), nullptr );
}
