// ##################################################################################################
//
//    Caffa
//    Copyright (C) 3D-Radar AS
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
struct JsonDataType
{
    static nlohmann::json type()
    {
        auto object    = nlohmann::json::object();
        object["type"] = "object";

        return object;
    }
};

template <typename DataType>
struct JsonDataType<std::vector<DataType>>
{
    static nlohmann::json type()
    {
        auto object     = nlohmann::json::object();
        object["type"]  = "array";
        object["items"] = JsonDataType<DataType>::type();

        return object;
    }
};

template <std::floating_point DataType>
struct JsonDataType<DataType>
{
    static nlohmann::json type()
    {
        auto object    = nlohmann::json::object();
        object["type"] = "number";
        return object;
    }
};

template <std::integral DataType>
struct JsonDataType<DataType>
{
    static nlohmann::json type()
    {
        auto object    = nlohmann::json::object();
        object["type"] = "integer";
        return object;
    }
};

template <>
struct JsonDataType<bool>
{
    static nlohmann::json type()
    {
        auto object    = nlohmann::json::object();
        object["type"] = "boolean";
        return object;
    }
};

template <>
struct JsonDataType<std::string>
{
    static nlohmann::json type()
    {
        auto object    = nlohmann::json::object();
        object["type"] = "string";
        return object;
    }
};

} // namespace caffa