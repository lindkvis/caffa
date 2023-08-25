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
#include "cafRestServer.h"

#include "cafFactory.h"
#include "cafLogger.h"
#include "cafRestServiceInterface.h"
#include "cafRestSession.h"
#include "cafStringTools.h"

#include <nlohmann/json.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/beast/version.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace caffa::rpc;

RestServer::RestServer( net::io_context&                        ioc,
                        std::shared_ptr<ssl::context>           sslContext,
                        tcp::endpoint                           endpoint,
                        const std::string&                      docRoot,
                        std::shared_ptr<const RestAuthenticator> authenticator )
    : m_ioContext( ioc )
    , m_sslContext( sslContext )
    , m_acceptor( net::make_strand( ioc ) )
    , m_docRoot( docRoot )
    , m_authenticator( authenticator )
{
    for ( auto key : RestServiceFactory::instance()->allKeys() )
    {
        auto service    = std::shared_ptr<RestServiceInterface>( RestServiceFactory::instance()->create( key ) );
        m_services[key] = service;
    }

    beast::error_code ec;

    // Open the acceptor
    m_acceptor.open( endpoint.protocol(), ec );
    if ( ec )
    {
        throw std::runtime_error( "Failed to open acceptor: " + ec.message() );
    }

    // Allow address reuse
    m_acceptor.set_option( net::socket_base::reuse_address( true ), ec );
    if ( ec )
    {
        throw std::runtime_error( "Failed to set socket option: " + ec.message() );
    }

    // Bind to the server address
    m_acceptor.bind( endpoint, ec );
    if ( ec )
    {
        throw std::runtime_error( "Failed to bind server address: " + ec.message() );
    }

    // Start listening for connections
    m_acceptor.listen( net::socket_base::max_listen_connections, ec );
    if ( ec )
    {
        throw std::runtime_error( "Failed to listen to socket: " + ec.message() );
    }
}

// Start accepting incoming connections
void RestServer::run()
{
    accept();
}

void RestServer::quit()
{
}

void RestServer::accept()
{
    // The new connection gets its own strand
    m_acceptor.async_accept( net::make_strand( m_ioContext ),
                             beast::bind_front_handler( &RestServer::onAccept, shared_from_this() ) );
}

void RestServer::onAccept( beast::error_code ec, tcp::socket socket )
{
    if ( ec )
    {
        throw std::runtime_error( "Failed to accept client connection: " + ec.message() );
    }
    else
    {
        // Create the session and run it
        auto session =
            std::make_shared<DetectSession>( std::move( socket ), m_sslContext, m_docRoot, m_services, m_authenticator );
        if ( session )
        {
            session->run();
        }
    }

    // Accept another connection
    accept();
}