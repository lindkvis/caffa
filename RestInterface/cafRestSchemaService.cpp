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

std::pair<http::status, std::string> RestSchemaService::perform( http::verb                    verb,
                                                                 const std::list<std::string>& path,
                                                                 const nlohmann::json&         arguments,
                                                                 const nlohmann::json&         metaData )
{
    if ( verb != http::verb::get )
    {
        return std::make_pair( http::status::bad_request, "Only GET requests are allowed for schema queries" );
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
        return std::make_pair( http::status::unauthorized, "No session provided" );
    }

    if ( path.empty() )
    {
        return getAllSchemas();
    }

    auto factory = DefaultObjectFactory::instance();
    auto object  = factory->create( path.front() );

    if ( !object )
    {
        return std::make_pair( http::status::not_found, "No such class" );
    }

    auto reducedPath = path;
    reducedPath.pop_front();
    if ( reducedPath.empty() )
    {
        return std::make_pair( http::status::ok, createJsonSchemaFromProjectObject( object.get() ).dump() );
    }
    return getFieldSchema( object.get(), reducedPath.front() );
}

std::pair<http::status, std::string> RestSchemaService::getAllSchemas()
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

    return std::make_pair( http::status::ok, root.dump() );
}

std::pair<http::status, std::string> RestSchemaService::getFieldSchema( const caffa::ObjectHandle* object,
                                                                        const std::string&         fieldName )
{
    auto field = object->findField( fieldName );
    if ( !field ) return std::make_pair( http::status::not_found, "Field does not exist" );

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_pair( http::status::forbidden, "Field is not remote writable" );

    auto ioCapability = field->capability<caffa::FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_pair( http::status::forbidden,
                               "Field " + field->keyword() + " found, but it has no JSON capability" );
    }

    caffa::JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
    serializer.setSerializationType( Serializer::SerializationType::SCHEMA );

    nlohmann::json json;
    ioCapability->writeToJson( json, serializer );
    return std::make_pair( http::status::ok, json.dump() );
}
