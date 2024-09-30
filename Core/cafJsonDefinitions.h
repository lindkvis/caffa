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

#include <boost/json.hpp>

#include <chrono>
#include <string>

namespace caffa::json
{
using array  = boost::json::array;
using object = boost::json::object;
using value  = boost::json::value;

inline value parse( const std::string& string )
{
    value jsonValue;
    try
    {
        jsonValue = boost::json::parse( string );
    }
    catch ( const std::exception& )
    {
        // Quote the input and try again
        const std::string quotedString = "\"" + caffa::StringTools::trim( string ) + "\"";
        try
        {
            jsonValue = boost::json::parse( quotedString );
        }
        catch ( const std::exception& )
        {
            throw;
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
    return boost::json::value_to<T>( value );
}

template <typename T>
value to_json( T&& typedValue )
{
    return boost::json::value_from<T>( static_cast<T&&>( typedValue ) );
}

} // namespace caffa::json
