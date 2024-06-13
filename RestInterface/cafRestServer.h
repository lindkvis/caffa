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
// ##################################################################################################
#pragma once

#include "cafRestServiceInterface.h"

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <thread>

namespace httplib
{
class Server;
}

namespace caffa::rpc
{
class RestAuthenticator;

/**
 * @brief HTTP Server
 *
 */
class RestServer
{
public:
    using HttpVerb = RestServiceInterface::HttpVerb;

    RestServer( const std::string& clientHost, int threads, int port, std::shared_ptr<const RestAuthenticator> authenticator );
    ~RestServer();
    // Start accepting incoming connections
    void run();
    void quit();
    bool running() const;
    void waitUntilReady() const;

private:
    using Handler = std::function<void( const httplib::Request& request, httplib::Response& response )>;
    void handleRequest( HttpVerb verb, const httplib::Request& request, httplib::Response& response );

private:
    std::string m_clientHost;
    int         m_port;
    int         m_threads;

    std::shared_ptr<httplib::Server> m_httpServer;

    std::map<std::string, std::shared_ptr<RestServiceInterface>> m_services;
    std::shared_ptr<const RestAuthenticator>                     m_authenticator;
};
} // namespace caffa::rpc