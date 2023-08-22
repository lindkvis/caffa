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
class RestObjectService : public RestServiceInterface
{
public:
    ServiceResponse perform( http::verb                    verb,
                             const std::list<std::string>& path,
                             const nlohmann::json&         arguments,
                             const nlohmann::json&         metaData ) override;

    bool requiresAuthentication( const std::list<std::string>& path ) const override;

private:
    static caffa::Document*     document( const std::string& documentId, const caffa::Session* session );
    static caffa::ObjectHandle* findObject( const std::string& uuid, const caffa::Session* session );
    static ServiceResponse      documents( const caffa::Session* session, bool skeleton );

    static ServiceResponse perform( http::verb                    verb,
                                    caffa::ObjectHandle*          object,
                                    const std::list<std::string>& path,
                                    const nlohmann::json&         arguments,
                                    bool                          skeleton,
                                    bool                          replace );

    static std::pair<caffa::ObjectAttribute*, int64_t> findFieldOrMethod( caffa::ObjectHandle*          object,
                                                                          const std::list<std::string>& path );

    static ServiceResponse getFieldValue( const caffa::FieldHandle* fieldHandle, int64_t index, bool skeleton );
    static ServiceResponse putFieldValue( caffa::FieldHandle*   fieldHandle,
                                          int64_t               index,
                                          const nlohmann::json& arguments,
                                          bool                  replace = false );
    static ServiceResponse deleteChildObject( caffa::FieldHandle* field, int64_t index, const nlohmann::json& arguments );
};

} // namespace caffa::rpc
