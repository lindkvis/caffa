//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
#pragma once

#include <iostream>
#include <list>
#include <memory>
#include <mutex>

namespace caffa::rpc
{
class ServerImpl;

class Server
{
public:
    /**
     * Constructor. Provide a path to a serverCertFile and serverKeyFile to enable SSL/TLS
     * @param port Port number (default 50000)
     * @param serverCertFile File path to a server certificate
     * @param serverKeyFile File path to a server private key
     */
    Server( int                port           = 50000,
            const std::string& serverCertFile = "",
            const std::string& serverKeyFile  = "",
            const std::string& caCertFile     = "" );
    ~Server();

    void run();
    void quit();
    bool quitting() const;

    int  port() const;
    bool running() const;

private:
    void initialize();
    void cleanup();

private:
    Server( const Server& ) = delete;

    ServerImpl* m_serverImpl;
};
} // namespace caffa::rpc
