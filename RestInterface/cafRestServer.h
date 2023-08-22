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
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <list>
#include <memory>
#include <string>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http  = beast::http; // from <boost/beast/http.hpp>
namespace net   = boost::asio; // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace caffa::rpc
{
class WebAuthenticator
{
public:
    virtual ~WebAuthenticator() = default;

    virtual bool authenticate( const std::string& authorizationHeader ) const = 0;
};

/**
 * @brief HTTP Session
 *
 */
class WebSession : public std::enable_shared_from_this<WebSession>
{
    struct SendLambda;

public:
    // Take ownership of the stream
    WebSession( tcp::socket&&                                                       socket,
                const std::string&                                                  docRoot,
                const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                std::shared_ptr<const WebAuthenticator>                             authenticator );

    // Start the asynchronous operation
    void run();

    void read();
    void onRead( beast::error_code ec, std::size_t bytesTransferred );
    void onWrite( bool close, beast::error_code ec, std::size_t bytesTransferred );
    void close();

    std::shared_ptr<RestServiceInterface>                        service( const std::string& key ) const;
    std::map<std::string, std::shared_ptr<RestServiceInterface>> services() const;

    std::string peer() const;

    bool authenticate( const std::string& authorizationHeader ) const;

private:
    beast::tcp_stream                m_stream;
    beast::flat_buffer               m_buffer;
    std::string                      m_docRoot;
    http::request<http::string_body> m_request;
    std::shared_ptr<void>            m_result;
    std::shared_ptr<SendLambda>      m_lambda;

    RestServiceInterface::CleanupCallback m_cleanupCallback;

    std::map<std::string, std::shared_ptr<RestServiceInterface>> m_services;

    std::shared_ptr<const WebAuthenticator> m_authenticator;
};

/**
 * @brief HTTP Server
 *
 */
class RestServer : public std::enable_shared_from_this<RestServer>
{
public:
    RestServer( net::io_context&                        ioc,
                tcp::endpoint                           endpoint,
                const std::string&                      docRoot,
                std::shared_ptr<const WebAuthenticator> authenticator );

    // Start accepting incoming connections
    void run();

    void quit();

private:
    void accept();
    void onAccept( beast::error_code ec, tcp::socket socket );

private:
    net::io_context& m_ioContext;
    tcp::acceptor    m_acceptor;
    std::string      m_docRoot;

    std::map<std::string, std::shared_ptr<RestServiceInterface>> m_services;
    std::shared_ptr<const WebAuthenticator>                      m_authenticator;
};
} // namespace caffa::rpc