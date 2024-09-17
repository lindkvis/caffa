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
#include "cafSession.h"

#include <string>
#include <utility>

namespace caffa
{
class Document;
class FieldHandle;
class MethodHandle;
class ObjectHandle;
class Session;
} // namespace caffa

namespace caffa::rpc
{
/**
 * @brief REST-service answering request searching for arbitrary Objects in project tree
 *
 */
class RestObjectService final : public RestServiceInterface
{
public:
    RestObjectService();

    ServiceResponse perform( http::verb             verb,
                             std::list<std::string> path,
                             const json::object&    queryParams,
                             const json::value&     body ) override;

    [[nodiscard]] bool requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const override;
    [[nodiscard]] bool requiresSession( http::verb verb, const std::list<std::string>& path ) const override;

    [[nodiscard]] std::map<std::string, json::object> servicePathEntries() const override;
    [[nodiscard]] std::map<std::string, json::object> serviceComponentEntries() const override;

private:
    static json::object anyObjectResponseContent();
    static json::object anyFieldResponseContent();

    static ServiceResponse object( http::verb                    verb,
                                   const std::list<std::string>& pathArguments,
                                   const json::object&           queryParams,
                                   const json::value&            body );

    static ServiceResponse getFieldValue( const caffa::FieldHandle* field, int64_t index, bool skeleton );
    static ServiceResponse replaceFieldValue( caffa::FieldHandle* field, int64_t index, const json::value& body );
    static ServiceResponse insertFieldValue( caffa::FieldHandle* field, int64_t index, const json::value& body );
    static ServiceResponse deleteFieldValue( caffa::FieldHandle* field, int64_t index );

    static ServiceResponse performFieldOrMethodOperation( http::verb                    verb,
                                                          const std::list<std::string>& pathArguments,
                                                          const json::object&           queryParams,
                                                          const json::value&            body );

    static caffa::SessionMaintainer findSession( const json::object& queryParams );

    static std::unique_ptr<RestAction> createObjectGetAction( const std::list<RestParameter*>& parameters );
    static std::unique_ptr<RestAction> createFieldOrMethodAction( http::verb                       verb,
                                                                  const std::string&               description,
                                                                  const std::string&               name,
                                                                  const std::list<RestParameter*>& parameters );

private:
    std::unique_ptr<RestPathEntry> m_requestPathRoot;
};

} // namespace caffa::rpc
