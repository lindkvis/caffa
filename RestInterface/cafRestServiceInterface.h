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

#include "cafFactory.h"

#include <nlohmann/json.hpp>

#include <boost/beast/http.hpp>

#include <functional>
#include <list>
#include <map>
#include <tuple>

namespace http = boost::beast::http;

namespace caffa::rpc
{
class AbstractRestCallback;

/**
 * The base interface for all REST-services. Implement this to create a new service.
 */
class RestServiceInterface
{
public:
    // Callback to be called after sending response. Can be nullptr
    using CleanupCallback = std::function<void()>;
    using ServiceResponse = std::tuple<http::status, std::string, CleanupCallback>;

    virtual ~RestServiceInterface() = default;

    virtual ServiceResponse perform( http::verb                    verb,
                                     const std::list<std::string>& path,
                                     const nlohmann::json&         arguments,
                                     const nlohmann::json&         metaData ) = 0;

    virtual bool requiresAuthentication( const std::list<std::string>& path ) const = 0;
};

typedef caffa::Factory<RestServiceInterface, std::string> RestServiceFactory;
} // namespace caffa::rpc
