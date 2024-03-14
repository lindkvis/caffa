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

#include <chrono>
#include <functional>
#include <list>
#include <map>
#include <mutex>
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
    static const std::string HTTP_OK;
    static const std::string HTTP_ACCEPTED;
    static const std::string HTTP_FORBIDDEN;
    static const std::string HTTP_TOO_MANY_REQUESTS;
    static const std::string HTTP_NOT_FOUND;

    // Callback to be called after sending response. Can be nullptr
    using CleanupCallback = std::function<void()>;
    using ServiceResponse = std::tuple<http::status, std::string, CleanupCallback>;

    virtual ~RestServiceInterface() = default;

    virtual ServiceResponse perform( http::verb             verb,
                                     std::list<std::string> path,
                                     const nlohmann::json&  queryParams,
                                     const nlohmann::json&  body ) = 0;

    /**
     * @brief Check whether the service requires authentication
     * Any service implementing the interface makes its own decision on this.
     * Unauthenticated services are still subject to rate limiting
     *
     * @param verb The verb used to access the service
     * @param path The path used to access the service
     * @return true if the service requires authentication for this verb and path
     * @return false if the service does not require authentication for this verb and path
     */
    virtual bool requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const = 0;

    /**
     * @brief Check whether the service requires a valid session
     * Any service implementing the interface makes its own decision on this.
     * Unauthenticated services are still subject to rate limiting
     *
     * @param verb The verb used to access the service
     * @param path The path used to access the service
     * @return true if the service requires a valid session for this verb and path
     * @return false if the service does not requires a valid session for this verb and path
     */
    virtual bool requiresSession( http::verb verb, const std::list<std::string>& path ) const = 0;

    /**
     * @brief Create a plain text OpenAPI error response
     *
     * @return nlohmann::json a response object
     */
    static nlohmann::json plainErrorResponse();

    /**
     * @brief Get the basic OpenAPI service schemas required for all services
     *
     * @return std::map<std::string, nlohmann::json>
     */
    static std::map<std::string, nlohmann::json> basicServiceSchemas();

    /**
     * @brief Entries into the OpenAPI paths objects
     *
     * @return std::map<std::string, nlohmann::json>
     */
    virtual std::map<std::string, nlohmann::json> servicePathEntries() const = 0;

    /**
     * @brief Entries into the OpenAPI components objects
     *
     * @return std::map<std::string, nlohmann::json>
     */
    virtual std::map<std::string, nlohmann::json> serviceComponentEntries() const = 0;

    static bool refuseDueToTimeLimiter();

private:
    static std::list<std::chrono::steady_clock::time_point> s_requestTimes;
    static std::mutex                                       s_requestMutex;
};

typedef caffa::Factory<RestServiceInterface, std::string> RestServiceFactory;
} // namespace caffa::rpc
