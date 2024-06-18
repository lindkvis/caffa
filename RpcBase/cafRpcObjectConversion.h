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

#include <nlohmann/json.hpp>

#include <string>

namespace caffa
{
class FieldHandle;
class ObjectHandle;
class Session;
} // namespace caffa

namespace caffa::rpc
{
bool                 fieldIsScriptReadable( const caffa::FieldHandle* fieldHandle );
caffa::ObjectHandle* findCafObjectFromJsonObject( const caffa::Session* session, const std::string& jsonObject );
caffa::ObjectHandle* findCafObjectFromUuid( const caffa::Session* session, const std::string& objectUuid );

nlohmann::json createJsonSchemaFromProjectObject( const caffa::ObjectHandle* source );
nlohmann::json createJsonSkeletonFromProjectObject( const caffa::ObjectHandle* source );
nlohmann::json createJsonFromProjectObject( const caffa::ObjectHandle* source );

} // namespace caffa::rpc
