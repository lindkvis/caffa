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
//    Based on example code from Boost Beast:
//    Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
//    Distributed under the Boost Software License, Version 1.0. (See accompanying
//    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//    Official repository: https://github.com/boostorg/beast
//
// ##################################################################################################
#pragma once

#include "cafLogger.h"
#include "cafRestAuthenticator.h"
#include "cafRestServiceInterface.h"
#include "cafStringEncoding.h"
#include "cafStringTools.h"

#include <utility> // Workaround for boost/asio not being a self-contained include

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/regex.hpp>

#include <nlohmann/json.hpp>

#include <list>
#include <memory>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http  = beast::http; // from <boost/beast/http.hpp>
namespace net   = boost::asio; // from <boost/asio.hpp>
namespace ssl   = boost::asio::ssl; // from <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace caffa::rpc
{
// The function object is used to send an HTTP message.
template <class Derived>
struct SendLambda;

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template <class Derived>
class RestSession
{
public:
    // Take ownership of the buffer
    RestSession( beast::flat_buffer                                                  buffer,
                 const std::string&                                                  docRoot,
                 const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                 std::shared_ptr<const RestAuthenticator>                            authenticator );

    void read();
    void onRead( beast::error_code ec, std::size_t bytes_transferred );
    void onWrite( bool close, beast::error_code ec, std::size_t bytes_transferred );
    bool authenticate( const std::string& authorisationHeader ) const;

private:
    friend struct SendLambda<Derived>;

    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived& derived();

private:
    std::string                           m_docRoot;
    http::request<http::string_body>      m_request;
    std::shared_ptr<void>                 m_result;
    std::shared_ptr<SendLambda<Derived>>  m_lambda;
    RestServiceInterface::CleanupCallback m_cleanupCallback;

    const std::map<std::string, std::shared_ptr<RestServiceInterface>>& m_services;
    std::shared_ptr<const RestAuthenticator>                            m_authenticator;

protected:
    beast::flat_buffer m_buffer;
};

// Handles a plain HTTP connection
class PlainRestSession : public RestSession<PlainRestSession>, public std::enable_shared_from_this<PlainRestSession>
{
    beast::tcp_stream m_stream;

public:
    // Create the session
    PlainRestSession( tcp::socket&&                                                       socket,
                      beast::flat_buffer                                                  buffer,
                      const std::string&                                                  docRoot,
                      const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                      std::shared_ptr<const RestAuthenticator>                            authenticator );

    // Called by the base class
    beast::tcp_stream& stream();

    // Start the asynchronous operation
    void run();
    void sendEof();
};

// Handles an SSL HTTP connection
class SslRestSession : public RestSession<SslRestSession>, public std::enable_shared_from_this<SslRestSession>
{
    beast::ssl_stream<beast::tcp_stream> m_stream;

public:
    // Create the session
    SslRestSession( tcp::socket&&                                                       socket,
                    ssl::context&                                                       ctx,
                    beast::flat_buffer                                                  buffer,
                    const std::string&                                                  docRoot,
                    const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                    std::shared_ptr<const RestAuthenticator>                            authenticator );

    // Called by the base class
    beast::ssl_stream<beast::tcp_stream>& stream();

    // Start the asynchronous operation
    void run();
    void onHandshake( beast::error_code ec, std::size_t bytes_used );
    void sendEof();
    void onShutdown( beast::error_code ec );
};

//------------------------------------------------------------------------------

// Detects SSL handshakes
class DetectSession : public std::enable_shared_from_this<DetectSession>
{
public:
    DetectSession( tcp::socket&&                                                       socket,
                   std::shared_ptr<ssl::context>                                       sslContext,
                   const std::string&                                                  docRoot,
                   const std::map<std::string, std::shared_ptr<RestServiceInterface>>& services,
                   std::shared_ptr<const RestAuthenticator>                            authenticator );

    // Launch the detector
    void run();

    // Launch the detector
    void onDetect( beast::error_code ec, bool result );

private:
    beast::tcp_stream                                                   m_stream;
    std::shared_ptr<ssl::context>                                       m_sslContext;
    std::string                                                         m_docRoot;
    beast::flat_buffer                                                  m_buffer;
    const std::map<std::string, std::shared_ptr<RestServiceInterface>>& m_services;
    std::shared_ptr<const RestAuthenticator>                            m_authenticator;
};

} // namespace caffa::rpc
