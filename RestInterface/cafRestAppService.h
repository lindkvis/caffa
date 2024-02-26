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

#include <list>
#include <utility>
#include <vector>

namespace caffa::rpc
{
class RestAppService : public RestServiceInterface
{
public:
    RestAppService();

    ServiceResponse perform( http::verb             verb,
                             std::list<std::string> path,
                             const nlohmann::json&  queryParams,
                             const nlohmann::json&  body ) override;

    bool requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const override;
    bool requiresSession( http::verb verb, const std::list<std::string>& path ) const override;

    std::map<std::string, nlohmann::json> servicePathEntries() const override;
    std::map<std::string, nlohmann::json> serviceComponentEntries() const override;

private:
    using ServiceCallback = std::function<ServiceResponse( http::verb verb, const nlohmann::json&, const nlohmann::json& )>;

    static ServiceResponse
        info( const std::list<std::string>& pathArguments, const nlohmann::json& queryParams, const nlohmann::json& body );
    static ServiceResponse
        quit( const std::list<std::string>& pathArguments, const nlohmann::json& queryParams, const nlohmann::json& body );

    std::unique_ptr<RestPathEntry> m_requestPathRoot;
};
} // namespace caffa::rpc
