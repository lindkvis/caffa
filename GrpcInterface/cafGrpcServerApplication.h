// ##################################################################################################
//
//    Caffa
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

#include "cafGrpcServer.h"
#include "cafNotNull.h"
#include "cafRpcServerApplication.h"
#include "cafSession.h"

#include <memory>

namespace caffa
{

namespace rpc
{

    class GrpcServerApplication : public ServerApplication
    {
    public:
        /**
         * Constructor. Provide a path to a serverCertFile and serverKeyFile to enable SSL/TLS
         * @param port Port number
         * @param serverCertFile File path to a server certificate
         * @param serverKeyFile File path to a server private key
         */
        GrpcServerApplication( int                portNumber,
                               const std::string& serverCertFile = "",
                               const std::string& serverKeyFile  = "",
                               const std::string& caCertFile     = "" );
        static GrpcServerApplication* instance();

        int  portNumber() const override;
        void run() override;
        void quit() override;

        bool running() const;

    private:
        std::unique_ptr<GrpcServer> m_server;
    };
} // namespace rpc
} // namespace caffa