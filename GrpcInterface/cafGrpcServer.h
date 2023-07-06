// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2020-2021 Gaute Lindkvist
//    Copyright (C) 2021- Kontur AS
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

#include "cafRpcServer.h"

#include <iostream>
#include <list>
#include <memory>
#include <mutex>

namespace caffa::rpc
{
class GrpcServerImpl;

class GrpcServer : public Server
{
public:
    /**
     * Constructor. Provide a path to a serverCertFile and serverKeyFile to enable SSL/TLS
     * @param port Port number (default 50000)
     * @param serverCertFile File path to a server certificate
     * @param serverKeyFile File path to a server private key
     */
    GrpcServer( int                port           = 50000,
                const std::string& serverCertFile = "",
                const std::string& serverKeyFile  = "",
                const std::string& caCertFile     = "" );
    ~GrpcServer() override;

    void run() override;
    void quit() override;
    bool running() const override;

    bool quitting() const;
    int  port() const;

private:
    void initialize();
    void cleanup();

private:
    GrpcServer( const GrpcServer& ) = delete;

    GrpcServerImpl* m_serverImpl;
};
} // namespace caffa::rpc
