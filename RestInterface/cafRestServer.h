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

#include <utility> // Workaround for boost/asio not being a self-contained include

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

#include <memory>
#include <string>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http  = beast::http; // from <boost/beast/http.hpp>
namespace net   = boost::asio; // from <boost/asio.hpp>
namespace ssl   = boost::asio::ssl; // from <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace caffa::rpc
{
class RestAuthenticator;

/**
 * @brief HTTP Server
 *
 */
class RestServer : public std::enable_shared_from_this<RestServer>
{
public:
    RestServer( net::io_context&                         ioc,
                std::shared_ptr<ssl::context>            sslContext,
                tcp::endpoint                            endpoint,
                const std::string&                       docRoot,
                std::shared_ptr<const RestAuthenticator> authenticator );

    // Start accepting incoming connections
    void run();

    void quit();

private:
    void accept();
    void onAccept( beast::error_code ec, tcp::socket socket );

private:
    net::io_context&              m_ioContext;
    std::shared_ptr<ssl::context> m_sslContext;
    tcp::acceptor                 m_acceptor;
    std::string                   m_docRoot;

    std::map<std::string, std::shared_ptr<RestServiceInterface>> m_services;
    std::shared_ptr<const RestAuthenticator>                     m_authenticator;
};
} // namespace caffa::rpc