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

#include <boost/asio.hpp>

#include <memory>
#include <vector>

namespace net = boost::asio; // from <boost/asio.hpp>

namespace caffa::rpc
{
class RestServer;
class RestServerApplication : public caffa::rpc::ServerApplication
{
public:
    /**
     * Constructor.
     * @param portNumber Port number
     * @param threads The number of accept threads
     */
    RestServerApplication( unsigned short portNumber, int threads );
    static RestServerApplication* instance();

    int  portNumber() const override;
    void run() override;
    void quit() override;
    bool running() const override;

private:
    unsigned short              m_portNumber;
    int                         m_threads;
    net::io_context             m_ioContext;
    std::shared_ptr<RestServer> m_server;
};
} // namespace caffa::rpc