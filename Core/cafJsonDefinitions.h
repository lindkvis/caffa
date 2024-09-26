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

template <class T,
          class D1 = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_protected>,
          class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
          class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value>>

void tag_invoke( boost::json::value_from_tag const&, boost::json::value& v, T const& t )
{
    auto& obj = v.emplace_object();

    boost::mp11::mp_for_each<D1>( [&]( auto D ) { obj[D.name] = boost::json::value_from( t.*D.pointer ); } );
}
} // namespace caffa::json

namespace boost::json
{

/**
 * Serialisers for chrono time points
 */
template <typename Clock, typename Duration>
void tag_invoke( value_from_tag, value& v, const std::chrono::time_point<Clock, Duration>& tp )
{
    v = value_from( std::chrono::duration_cast<Duration>( tp.time_since_epoch() ).count() );
}

template <typename Clock, typename Duration>
std::chrono::time_point<Clock, Duration> tag_invoke( value_to_tag<std::chrono::time_point<Clock, Duration>>, value const& v )
{
    Duration duration( v.as_int64() );
    return std::chrono::steady_clock::time_point( duration );
}

/**
 * Serialisers for chrono durations
 */
template <typename IntType, typename Period>
void tag_invoke( value_from_tag, value& v, std::chrono::duration<IntType, Period> const& duration )
{
    v = value_from<IntType>( static_cast<IntType>( duration.count() ) );
}

template <typename IntType, typename Period>
std::chrono::duration<IntType, Period> tag_invoke( value_to_tag<std::chrono::duration<IntType, Period>>, value const& v )
{
    return std::chrono::duration<IntType, Period>( value_to<IntType>( v ) );
}

} // namespace boost::json
