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
 * @brief REST-service answering request for explicit paths in the project tree
 *
 */
class RestDocumentService : public RestServiceInterface
{
public:
    RestDocumentService();

    ServiceResponse perform( http::verb             verb,
                             std::list<std::string> path,
                             const json::object&    queryParams,
                             const json::value&     body ) override;

    [[nodiscard]] bool requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const override;
    [[nodiscard]] bool requiresSession( http::verb verb, const std::list<std::string>& path ) const override;

    [[nodiscard]] std::map<std::string, json::object> servicePathEntries() const override;
    [[nodiscard]] std::map<std::string, json::object> serviceComponentEntries() const override;

private:
    static ServiceResponse document( const std::string&            documentId,
                                     http::verb                    verb,
                                     const std::list<std::string>& pathArguments,
                                     const json::object&           queryParams,
                                     const json::value&            body );
    static ServiceResponse documents( http::verb                    verb,
                                      const std::list<std::string>& pathArguments,
                                      const json::object&           queryParams,
                                      const json::value&            body );

    std::unique_ptr<RestPathEntry> m_requestPathRoot;
};

} // namespace caffa::rpc
