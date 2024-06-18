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
    static std::string name() { return "object"; }
};

template <>
struct PortableDataType<void>
{
    static std::string name() { return "void"; }
};

template <typename DataType>
struct PortableDataType<std::vector<DataType>>
{
    static std::string name() { return PortableDataType<DataType>::name() + "[]"; }
};

template <std::unsigned_integral DataType>
struct PortableDataType<DataType>
{
    static std::string name() { return "uint" + std::to_string( sizeof( DataType ) * 8 ); }
};

template <std::signed_integral DataType>
struct PortableDataType<DataType>
{
    static std::string name() { return "int" + std::to_string( sizeof( DataType ) * 8 ); }
};

template <>
struct PortableDataType<bool>
{
    static std::string name() { return "boolean"; }
};

template <>
struct PortableDataType<float>
{
    static std::string name() { return "float"; }
};

template <>
struct PortableDataType<double>
{
    static std::string name() { return "double"; }
};

template <>
struct PortableDataType<std::string>
{
    static std::string name() { return "string"; }
};

template <>
struct PortableDataType<std::chrono::steady_clock::time_point>
{
    static std::string name() { return "timestamp_ns"; }
};

template <>
struct PortableDataType<std::chrono::nanoseconds>
{
    static std::string name() { return "nanoseconds"; }
};

template <>
struct PortableDataType<std::chrono::microseconds>
{
    static std::string name() { return "microseconds"; }
};

template <>
struct PortableDataType<std::chrono::milliseconds>
{
    static std::string name() { return "milliseconds"; }
};

template <>
struct PortableDataType<std::chrono::seconds>
{
    static std::string name() { return "seconds"; }
};

} // namespace caffa
