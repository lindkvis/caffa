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

#include "cafNotNull.h"
#include "cafRpcServerApplication.h"
#include "cafSession.h"

#include <utility>

#include <atomic>
#include <memory>
#include <mutex>

namespace caffa::rpc
{
class RestServer;
class RestAuthenticator;

/**
 * A base class for a REST server application.
 * Sub-class to create your REST application.
 */
class RestServerApplication : public caffa::rpc::ServerApplication
{
public:
    /**
     * Constructor.
     *
     * @param portNumber Port number
     * @param threads The number of accept threads
     * @param authenticator The authenticator used to determine if a client is allowed to access the server.
     */
    RestServerApplication( const std::string&                       clientHost,
                           unsigned short                           portNumber,
                           int                                      threads,
                           std::shared_ptr<const RestAuthenticator> authenticator );
    static RestServerApplication* instance();

    int  portNumber() const override;
    void run() override;
    void quit() override;
    bool running() const override;
    void waitUntilReady() const override;

private:
    std::string                              m_clientHost;
    unsigned short                           m_portNumber;
    int                                      m_threads;
    std::shared_ptr<const RestAuthenticator> m_authenticator;
    std::shared_ptr<RestServer>              m_server;
    std::atomic<bool>                        m_threadRunning;
};
} // namespace caffa::rpc
