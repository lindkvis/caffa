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
/**
 * @brief REST-service answering request searching for arbitrary Objects in project tree
 *
 */
class RestObjectService : public RestServiceInterface
{
public:
    ServiceResponse perform( http::verb             verb,
                             std::list<std::string> path,
                             const nlohmann::json&  queryParams,
                             const nlohmann::json&  body ) override;

    bool requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const override;
    bool requiresSession( http::verb verb, const std::list<std::string>& path ) const override;

    std::map<std::string, nlohmann::json> servicePathEntries() const override;
    std::map<std::string, nlohmann::json> serviceComponentEntries() const override;

private:
    static caffa::ObjectHandle* findObject( const std::string& uuid, const caffa::Session* session );

    static ServiceResponse performFieldOperation( std::shared_ptr<caffa::Session> session,
                                                          http::verb                      verb,
                                                          caffa::ObjectHandle*            object,
                                                          const std::string&              keyword,
                                                          const nlohmann::json&           queryParams,
                                                          const nlohmann::json&           body );
    static ServiceResponse performMethodOperation( std::shared_ptr<caffa::Session> session,
                                                          http::verb                      verb,
                                                          caffa::ObjectHandle*            object,
                                                          const std::string&              keyword,
                                                          const nlohmann::json&           queryParams,
                                                          const nlohmann::json&           body );

    static ServiceResponse getFieldValue( const caffa::FieldHandle* fieldHandle, int64_t index, bool skeleton );
    static ServiceResponse replaceFieldValue( caffa::FieldHandle* fieldHandle, int64_t index, const nlohmann::json& body );
    static ServiceResponse insertFieldValue( caffa::FieldHandle* fieldHandle, int64_t index, const nlohmann::json& body );
    static ServiceResponse deleteFieldValue( caffa::FieldHandle* field, int64_t index );
};

} // namespace caffa::rpc
