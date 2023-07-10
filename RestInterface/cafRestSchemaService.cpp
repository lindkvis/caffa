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

    auto factory = DefaultObjectFactory::instance();

    if ( path.empty() )
    {
        auto array = nlohmann::json::array();

        for ( auto className : factory->classes() )
        {
            array.push_back( className );
        }
        return std::make_pair( http::status::ok, array.dump() );
    }

    auto object = factory->create( path.front() );

    if ( !object )
    {
        return std::make_pair( http::status::not_found, "No such class" );
    }

    return std::make_pair( http::status::ok, createJsonSchemaFromProjectObject( object.get() ).dump() );
}
