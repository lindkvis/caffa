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

#include <boost/json.hpp>

#include <string>

namespace caffa::json
{
using array  = boost::json::array;
using object = boost::json::object;
using value  = boost::json::value;

bool        isParsableJson( const std::string& str );
value       parse( const std::string& string );
std::string dump( const value& value );

template <typename T>
T from_json( const value& value )
{
    T result = boost::json::value_to<T>( value );
    return result;
}

template <typename T>
value to_json( T&& typedValue )
{
    return boost::json::value_from<T>( static_cast<T&&>( typedValue ) );
}

} // namespace caffa::json
