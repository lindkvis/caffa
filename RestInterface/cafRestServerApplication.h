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

#include "cafRpcServerApplication.h"

#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>

#include <atomic>
#include <memory>

namespace net = boost::asio; // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

namespace caffa::rpc
{
class RestServer;
class RestAuthenticator;

/**
 * A base class for a REST server application.
 * Subclass to create your REST application.
 */
class RestServerApplication : public caffa::rpc::ServerApplication
{
public:
    /**
     * Constructor.
     *
     * @param clientHost
     * @param portNumber Port number
     * @param threads The number of accept threads
     * @param authenticator The authenticator used to determine if a client is allowed to access the server.
     */
    RestServerApplication( const std::string&                              clientHost,
                           unsigned short                                  portNumber,
                           int                                             threads,
                           const std::shared_ptr<const RestAuthenticator>& authenticator );
    static RestServerApplication* instance();

    int  portNumber() const override;
    void run() override;
    void quit() override;
    bool running() const override;

private:
    std::string                              m_clientHost;
    unsigned short                           m_portNumber;
    int                                      m_threads;
    net::io_context                          m_ioContext;
    std::shared_ptr<const RestAuthenticator> m_authenticator;
    std::shared_ptr<ssl::context>            m_sslContext;
    std::shared_ptr<RestServer>              m_server;
    std::atomic<bool>                        m_threadRunning;
};
} // namespace caffa::rpc
