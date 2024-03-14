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

#include "cafRestRequest.h"
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
    RestObjectService();

    ServiceResponse perform( http::verb             verb,
                             std::list<std::string> path,
                             const nlohmann::json&  queryParams,
                             const nlohmann::json&  body ) override;

    bool requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const override;
    bool requiresSession( http::verb verb, const std::list<std::string>& path ) const override;

    std::map<std::string, nlohmann::json> servicePathEntries() const override;
    std::map<std::string, nlohmann::json> serviceComponentEntries() const override;

private:
    static std::pair<caffa::ObjectHandle*, ServiceResponse> findObject( std::list<std::string>& pathArguments,
                                                                        const nlohmann::json&   queryParam );

    static nlohmann::json anyObjectResponseContent();
    static nlohmann::json anyFieldResponseContent();

    static ServiceResponse object( http::verb                    verb,
                                   const std::list<std::string>& pathArguments,
                                   const nlohmann::json&         queryParams,
                                   const nlohmann::json&         body );

    static ServiceResponse getFieldValue( const caffa::FieldHandle* fieldHandle, int64_t index, bool skeleton );
    static ServiceResponse replaceFieldValue( caffa::FieldHandle* fieldHandle, int64_t index, const nlohmann::json& body );
    static ServiceResponse insertFieldValue( caffa::FieldHandle* fieldHandle, int64_t index, const nlohmann::json& body );
    static ServiceResponse deleteFieldValue( caffa::FieldHandle* field, int64_t index );

    static ServiceResponse performFieldOperation( http::verb                    verb,
                                                  const std::list<std::string>& pathArguments,
                                                  const nlohmann::json&         queryParams,
                                                  const nlohmann::json&         body );

    static ServiceResponse executeMethod( http::verb                    verb,
                                          const std::list<std::string>& pathArguments,
                                          const nlohmann::json&         queryParams,
                                          const nlohmann::json&         body );

    std::unique_ptr<RestPathEntry> m_requestPathRoot;
};

} // namespace caffa::rpc
