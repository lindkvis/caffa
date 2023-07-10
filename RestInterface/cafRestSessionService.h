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

#include <utility>
#include <vector>

namespace caffa::rpc
{
class RestSessionService : public RestServiceInterface
{
public:
    std::pair<http::status, std::string> perform( http::verb                    verb,
                                                  const std::list<std::string>& path,
                                                  const nlohmann::json&         arguments,
                                                  const nlohmann::json&         metaData ) override;

private:
    using ServiceCallback =
        std::function<std::pair<http::status, std::string>( http::verb verb, const nlohmann::json&, const nlohmann::json& )>;

    std::map<std::string, ServiceCallback> callbacks() const;

    static std::pair<http::status, std::string>
        ready( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData );
    static std::pair<http::status, std::string>
        check( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData );
    static std::pair<http::status, std::string>
        change( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData );
    static std::pair<http::status, std::string>
        create( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData );
    static std::pair<http::status, std::string>
        destroy( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData );
    static std::pair<http::status, std::string>
        keepalive( http::verb verb, const nlohmann::json& arguments, const nlohmann::json& metaData );
};
} // namespace caffa::rpc
