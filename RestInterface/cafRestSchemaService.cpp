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

RestSchemaService::ServiceResponse RestSchemaService::perform( http::verb                    verb,
                                                               const std::list<std::string>& path,
                                                               const nlohmann::json&         queryParams,
                                                               const nlohmann::json&         body )
{
    if ( verb != http::verb::get )
    {
        return std::make_tuple( http::status::bad_request, "Only GET requests are allowed for schema queries", nullptr );
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

bool RestSchemaService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestSchemaService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

std::map<std::string, nlohmann::json> RestSchemaService::servicePathEntries() const
{
    return {};
}

std::map<std::string, nlohmann::json> RestSchemaService::serviceComponentEntries() const
{
    return {};
}

nlohmann::json RestSchemaService::getJsonForAllSchemas()
{
    auto factory = DefaultObjectFactory::instance();

    auto schemas = nlohmann::json::object();

    for ( auto className : factory->classes() )
    {
        auto object        = factory->create( className );
        schemas[className] = createJsonSchemaFromProjectObject( object.get() );
    }

    return schemas;
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

RestSchemaService::ServiceResponse RestSchemaService::getAllSchemas()
{
    return std::make_tuple( http::status::ok, getJsonForAllSchemas().dump(), nullptr );
}
