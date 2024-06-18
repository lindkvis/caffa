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
#include "cafObjectHandlePortableDataType.h"
#include "cafPortableDataType.h"

#include <nlohmann/json.hpp>

namespace caffa
{
template <class _Tp>
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

template <typename _Tp>
concept chrono_integral = is_chrono_integral<_Tp>::value;

template <typename _Tp>
concept supported_integral = std::integral<_Tp> || chrono_integral<_Tp>;

/**
 * The default
 */
template <typename DataType>
struct JsonDataType
{
    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["type"] = PortableDataType<DataType>::name();

        return object;
    }
};

template <>
struct JsonDataType<void>
{
    static nlohmann::json jsonType() { return nlohmann::json::object(); }
};

template <typename DataType>
struct JsonDataType<std::vector<DataType>>
{
    static nlohmann::json jsonType()
    {
        auto object     = nlohmann::json::object();
        object["type"]  = "array";
        object["items"] = JsonDataType<DataType>::jsonType();

        return object;
    }
};

template <supported_integral DataType>
struct JsonDataType<DataType>
{
    static nlohmann::json jsonType()
    {
        auto object      = nlohmann::json::object();
        object["type"]   = "integer";
        object["format"] = PortableDataType<DataType>::name();
        return object;
    }
};

template <std::floating_point DataType>
struct JsonDataType<DataType>
{
    static nlohmann::json jsonType()
    {
        auto object      = nlohmann::json::object();
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
    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["$ref"] = std::string( "#/components/object_schemas/" ) + DataType::classKeywordStatic();
        return object;
    }
};

/**
 * The portable type id for an ObjectHandle
 */
template <IsSharedPtr DataType>
struct JsonDataType<DataType>
{
    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["$ref"] = std::string( "#/components/object_schemas/" ) + DataType::element_type::classKeywordStatic();
        return object;
    }
};

template <typename EnumType>
struct JsonDataType<AppEnum<EnumType>>
{
    static nlohmann::json jsonType()
    {
        auto values = nlohmann::json::array();
        for ( auto entry : AppEnum<EnumType>::validLabels() )
        {
            values.push_back( entry );
        }
        auto object    = nlohmann::json::object();
        object["enum"] = values;
        return object;
    }
};

template <typename Enum>
void to_json( nlohmann::json& jsonValue, const AppEnum<Enum>& appEnum )
{
    std::stringstream stream;
    stream << appEnum;
    jsonValue = stream.str();
}

template <typename Enum>
void from_json( const nlohmann::json& jsonValue, AppEnum<Enum>& appEnum )
{
    std::stringstream stream( jsonValue.get<std::string>() );
    stream >> appEnum;
}

} // namespace caffa
