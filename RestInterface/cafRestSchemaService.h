// ##################################################################################################
//
//    Caffa
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
#pragma once

#include "cafRestServiceInterface.h"

#include <nlohmann/json.hpp>

#include <string>
#include <utility>

namespace caffa
{
class Document;
class FieldHandle;
class MethodHandle;
class ObjectAttribute;
class ObjectHandle;
class Session;
} // namespace caffa

namespace caffa::rpc
{
//==================================================================================================
//
// REST-service answering request searching for Objects in property tree
//
//==================================================================================================
class RestSchemaService : public RestServiceInterface
{
public:
    std::pair<http::status, std::string>        perform( http::verb                    verb,
                                                         const std::list<std::string>& path,
                                                         const nlohmann::json&         arguments,
                                                         const nlohmann::json&         metaData ) override;
    static std::pair<http::status, std::string> getFieldSchema( const caffa::ObjectHandle* object,
                                                                const std::string&         fieldName );

    static std::pair<http::status, std::string> getAllSchemas();
};

} // namespace caffa::rpc
