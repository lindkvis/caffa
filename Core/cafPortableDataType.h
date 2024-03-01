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

#include <nlohmann/json.hpp>

#include <chrono>
#include <concepts>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

namespace caffa
{
/**
 * The default
 */
template <typename DataType>
struct PortableDataType
{
    static std::string    name() { return "object"; }
    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["type"] = PortableDataType<DataType>::name();

        return object;
    }
};

template <>
struct PortableDataType<void>
{
    static std::string    name() { return "void"; }
    static nlohmann::json jsonType() { return nlohmann::json::object(); }
};

template <typename DataType>
struct PortableDataType<std::vector<DataType>>
{
    static std::string name() { return PortableDataType<DataType>::name() + "[]"; }

    static nlohmann::json jsonType()
    {
        auto object     = nlohmann::json::object();
        object["type"]  = "array";
        object["items"] = PortableDataType<DataType>::jsonType();

        return object;
    }
};

template <std::integral DataType>
struct PortableDataType<DataType>
{
    static std::string name() { return "int" + std::to_string( sizeof( DataType ) * 8 ); }

    static nlohmann::json jsonType()
    {
        auto object      = nlohmann::json::object();
        object["type"]   = "integer";
        object["format"] = PortableDataType<DataType>::name();
        return object;
    }
};

template <>
struct PortableDataType<bool>
{
    static std::string name() { return "boolean"; }

    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["type"] = name();
        return object;
    }
};

template <>
struct PortableDataType<float>
{
    static std::string name() { return "float"; }

    static nlohmann::json jsonType()
    {
        auto object      = nlohmann::json::object();
        object["type"]   = "number";
        object["format"] = name();
        return object;
    }
};

template <>
struct PortableDataType<double>
{
    static std::string name() { return "double"; }

    static nlohmann::json jsonType()
    {
        auto object      = nlohmann::json::object();
        object["type"]   = "number";
        object["format"] = name();

        return object;
    }
};

template <>
struct PortableDataType<std::string>
{
    static std::string name() { return "string"; }

    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["type"] = name();
        return object;
    }
};

template <>
struct PortableDataType<std::chrono::steady_clock::time_point>
{
    static std::string name() { return "timestamp_ns"; }

    static nlohmann::json jsonType()
    {
        auto object      = nlohmann::json::object();
        object["type"]   = "integer";
        object["format"] = PortableDataType<int64_t>::name();
        return object;
    }
};

} // namespace caffa