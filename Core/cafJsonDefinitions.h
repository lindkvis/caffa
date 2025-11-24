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
#pragma once

#include "cafStringTools.h"

#include "cafLogger.h"

#include <boost/json.hpp>

#include <chrono>
#include <string>

namespace caffa::json
{
using array  = boost::json::array;
using object = boost::json::object;
using value  = boost::json::value;

inline value parse( const std::string& string ) noexcept
{
    value jsonValue;
    try
    {
        auto isParsableJson = []( const std::string& str )
        {
            return str.empty() || str == "null" || str == "true" || str == "false" ||
                   ( str.front() == '"' && str.back() == '"' ) || std::isdigit( str.front() ) ||
                   ( str.front() == '-' && str.size() > 1 && std::isdigit( str[1] ) ) || str.front() == '{' ||
                   str.front() == '[';
        };
        if ( isParsableJson( string ) )
        {
            // Safe to parse
            jsonValue = boost::json::parse( string );
        }
        else
        {
            // Create trimmed and quoted string
            auto sanitise = []( const std::string& str ) { return "\"" + caffa::StringTools::trim( str ) + "\""; };

            jsonValue = boost::json::parse( sanitise( string ) );
        }
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to parse " << string << ": " << e.what() );

        // Quote the input and try again
        const std::string quotedString = "\"" + caffa::StringTools::trim( string ) + "\"";
        try
        {
            jsonValue = boost::json::parse( quotedString );
            CAFFA_WARNING( "Succeeded parsing " << quotedString );
        }
        catch ( const std::exception& )
        {
            CAFFA_ERROR( "Failed to parse " << quotedString );
        }
    }
    return jsonValue;
}

inline std::string dump( const value& value )
{
    return serialize( value );
}

template <typename T>
T from_json( const value& value )
{
    T result = boost::json::value_to<T>( value );
    // CAFFA_INFO( "Succeeded and got value: " << result );
    return result;
}

template <typename T>
value to_json( T&& typedValue )
{
    return boost::json::value_from<T>( static_cast<T&&>( typedValue ) );
}

} // namespace caffa::json
