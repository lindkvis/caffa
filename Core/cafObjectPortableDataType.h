// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2020- Kontur AS
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

#include "cafObjectHandle.h"
#include "cafPortableDataType.h"

namespace caffa
{
/**
 * The portable type id for an ObjectHandle
 */
template <DerivesFromObjectHandle DataType>
struct PortableDataType<DataType>
{
    static std::string    name() { return std::string( "object::" ) + DataType::classKeywordStatic(); }
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
struct PortableDataType<DataType>
{
    static std::string name() { return std::string( "object::" ) + DataType::element_type::classKeywordStatic(); }

    static nlohmann::json jsonType()
    {
        auto object    = nlohmann::json::object();
        object["$ref"] = std::string( "#/components/object_schemas/" ) + DataType::element_type::classKeywordStatic();
        return object;
    }
};
} // namespace caffa
