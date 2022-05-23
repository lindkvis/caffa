//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar AS
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

#include "cafGrpcApplication.h"
#include "cafGrpcServer.h"

#include <memory>

namespace caffa::rpc
{
class Session;

class ServerApplication : public Application
{
public:
    /**
     * Constructor. Provide a path to a serverCertFile and serverKeyFile to enable SSL/TLS
     * @param port Port number
     * @param serverCertFile File path to a server certificate
     * @param serverKeyFile File path to a server private key
     */
    ServerApplication( int                portNumber,
                       const std::string& serverCertFile = "",
                       const std::string& serverKeyFile  = "",
                       const std::string& caCertFile     = "" );
    static ServerApplication* instance();

    int  portNumber() const;
    void run();
    bool running() const;
    void quit();

    virtual Document*       document( const std::string& documentId, const std::string& sessionUuid = "" )       = 0;
    virtual const Document* document( const std::string& documentId, const std::string& sessionUuid = "" ) const = 0;
    virtual std::list<Document*>       documents( const std::string& sessionUuid = "" )                          = 0;
    virtual std::list<const Document*> documents( const std::string& sessionUuid = "" ) const                    = 0;

    virtual void resetToDefaultData() = 0;

    virtual Session* createSession()                                      = 0;
    virtual Session* getExistingSession( const std::string& sessionUuid ) = 0;
    virtual void     destroySession( const std::string& sessionUuid )     = 0;
    virtual void     keepAliveSession( const std::string& sessionUuid )   = 0;

private:
    virtual void onStartup() {}
    virtual void onShutdown() {}

private:
    std::unique_ptr<Server> m_server;
};
} // namespace caffa::rpc
