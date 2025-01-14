// ##################################################################################################
//
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
// ##################################################################################################
#pragma once

#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"

#include <boost/json.hpp>

#include <memory>

namespace caffa
{
/**
 * Serialisers for Caffa Objects
 */
template <DerivesFromObjectHandle ObjectPtrT>
void tag_invoke( boost::json::value_from_tag, boost::json::value& v, const std::shared_ptr<ObjectPtrT>& objectPtr )
{
    json::object jsonObject;
    JsonSerializer().writeObjectToJson( objectPtr.get(), jsonObject );
    v = jsonObject;
}

template <DerivesFromObjectHandle ObjectPtrT>
void tag_invoke( boost::json::value_from_tag, boost::json::value& v, const std::shared_ptr<const ObjectPtrT>& objectPtr )
{
    json::object jsonObject;
    JsonSerializer().writeObjectToJson( objectPtr.get(), jsonObject );
    v = jsonObject;
}

std::shared_ptr<ObjectHandle> tag_invoke( boost::json::value_to_tag<std::shared_ptr<ObjectHandle>>, const json::value& v );

template <class T,
          class D1 = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_protected>,
          class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
          class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value>>

void tag_invoke( boost::json::value_from_tag const&, boost::json::value& v, T const& t )
{
    auto& obj = v.emplace_object();

    boost::mp11::mp_for_each<D1>( [&]( auto D ) { obj[D.name] = boost::json::value_from( t.*D.pointer ); } );
}

} // namespace caffa

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
    return std::chrono::time_point<Clock, Duration>( duration );
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
