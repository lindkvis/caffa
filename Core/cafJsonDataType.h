// ##################################################################################################
//
//    Caffa
//    Copyright (C) Kontur AS
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

#include "cafAppEnum.h"
#include "cafJsonDefinitions.h"
#include "cafObjectHandlePortableDataType.h"
#include "cafPortableDataType.h"

#include <boost/json.hpp>

namespace caffa
{
template <class T>
struct is_chrono_integral : std::false_type
{
};

template <class Rep, class Period>
struct is_chrono_integral<std::chrono::duration<Rep, Period>> : std::true_type
{
};

template <class Clock, class Duration>
struct is_chrono_integral<std::chrono::time_point<Clock, Duration>> : std::true_type
{
};

template <typename T>
concept chrono_integral = is_chrono_integral<T>::value;

template <typename T>
concept supported_integral = ( std::integral<T> || chrono_integral<T> ) && !std::is_same_v<T, bool>;

/**
 * The default
 */
template <typename DataType>
struct JsonDataType
{
    static json::object jsonType()
    {
        json::object object;
        object["type"] = PortableDataType<DataType>::name();

        return object;
    }
};

template <>
struct JsonDataType<void>
{
    static json::object jsonType() { return json::object(); }
};

template <typename DataType>
struct JsonDataType<std::vector<DataType>>
{
    static json::object jsonType()
    {
        json::object object;
        object["type"]  = "array";
        object["items"] = JsonDataType<DataType>::jsonType();

        return object;
    }
};

template <typename DataType>
struct JsonDataType<std::map<std::string, DataType>>
{
    static json::object jsonType()
    {
        json::object object;
        json::object properties;
        properties["key"]    = { { "type", "string" } };
        properties["value"]  = { { "type", JsonDataType<DataType>::jsonType() } };
        object["properties"] = properties;
        object["type"]       = "object";
        return object;
    }
};

template <supported_integral DataType>
struct JsonDataType<DataType>
{
    static json::object jsonType()
    {
        json::object object;
        object["type"]   = "integer";
        object["format"] = PortableDataType<DataType>::name();
        return object;
    }
};

template <std::floating_point DataType>
struct JsonDataType<DataType>
{
    static json::object jsonType()
    {
        json::object object;
        object["type"]   = "number";
        object["format"] = PortableDataType<DataType>::name();
        return object;
    }
};

/**
 * The portable type id for an ObjectHandle
 */
template <DerivesFromObjectHandle DataType>
struct JsonDataType<DataType>
{
    static json::object jsonType()
    {
        json::object object;
        object["$ref"] = std::string( "#/components/object_schemas/" ) + std::string( DataType::classKeywordStatic() );
        return object;
    }
};

/**
 * The portable type id for an ObjectHandle
 */
template <IsSharedPtr DataType>
struct JsonDataType<DataType>
{
    static json::object jsonType()
    {
        json::object object;
        object["$ref"] = std::string( "#/components/object_schemas/" ) +
                         std::string( DataType::element_type::classKeywordStatic() );
        return object;
    }
};

template <typename EnumType>
struct JsonDataType<AppEnum<EnumType>>
{
    static json::object jsonType()
    {
        json::array values;
        for ( const auto& entry : AppEnum<EnumType>::validLabels() )
        {
            values.push_back( json::to_json( entry ) );
        }
        json::object object;
        object["enum"] = values;
        return object;
    }
};

template <typename Enum>
void tag_invoke( const boost::json::value_from_tag&, json::value& jsonValue, const AppEnum<Enum>& appEnum )
{
    std::stringstream stream;
    stream << appEnum;
    jsonValue = stream.str();
}

template <typename Enum>
AppEnum<Enum> tag_invoke( const boost::json::value_to_tag<AppEnum<Enum>>&, const json::value& jsonValue )
{
    if ( jsonValue.is_string() )
    {
        return AppEnum<Enum>( json::from_json<std::string>( jsonValue ) );
    }

    if ( jsonValue.is_int64() )
    {
        return AppEnum<Enum>( json::from_json<int64_t>( jsonValue ) );
    }

    throw std::runtime_error( "Invalid JSON value for creating AppEnum" );
}

} // namespace caffa
