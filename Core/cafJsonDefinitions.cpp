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
#include "cafJsonDefinitions.h"

#include "cafStringTools.h"

#include "cafLogger.h"

#include <boost/json.hpp>

#include <chrono>
#include <string>

namespace caffa::json
{
bool isParsableJson( const std::string& str )
{
    static const std::regex jsonNumber( R"(^-?(0|[1-9]\d*)(\.\d+)?([eE][+-]?\d+)?$)" );
    auto                    s = caffa::StringTools::trim( str );
    if ( s.empty() ) return false;
    if ( s == "null" || s == "true" || s == "false" ) return true;
    if ( s.front() == '"' && s.back() == '"' && s.size() >= 2 ) return true;
    if ( s.front() == '{' && s.back() == '}' && s.size() >= 2 ) return true;
    if ( s.front() == '[' && s.back() == ']' && s.size() >= 2 ) return true;
    if ( std::regex_match( s, jsonNumber ) ) return true;
    return false;
}

value parse( const std::string& string )
{
    boost::system::error_code ec;
    value                     jsonValue;
    if ( isParsableJson( string ) )
    {
        // Safe to parse
        jsonValue = boost::json::parse( string, ec );
    }
    else
    {
        // Create trimmed and quoted string
        auto sanitise = []( const std::string& str ) { return "\"" + caffa::StringTools::trim( str ) + "\""; };

        jsonValue = boost::json::parse( sanitise( string ), ec );
    }
    if ( ec != boost::system::errc::success )
    {
        CAFFA_ERROR( "Failed to parse " << string << ": " << ec.message() );
        return value{};
    }
    return jsonValue;
}

std::string dump( const value& value )
{
    return serialize( value );
}

} // namespace caffa::json
