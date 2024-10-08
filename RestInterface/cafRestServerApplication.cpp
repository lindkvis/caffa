// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- 3D-Radar AS
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
#include "cafRestServerApplication.h"

#include "cafRestAppService.h"
#include "cafRestAuthenticator.h"
#include "cafRestDocumentService.h"
#include "cafRestObjectService.h"
#include "cafRestOpenApiService.h"
#include "cafRestServer.h"
#include "cafRestServiceInterface.h"
#include "cafRestSessionService.h"

#include "cafLogger.h"

#include <thread>

#ifdef __APPLE__
using thread = std::thread;
#else
using thread = std::jthread;
#endif

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestServerApplication::RestServerApplication( const std::string&                              clientHost,
                                              const unsigned short                            portNumber,
                                              const int                                       threads,
                                              const std::shared_ptr<const RestAuthenticator>& authenticator )
    : ServerApplication( AppInfo::AppCapability::SERVER )
    , m_clientHost( clientHost )
    , m_portNumber( portNumber )
    , m_threads( threads )
    , m_ioContext( threads )
    , m_authenticator( authenticator )
    , m_threadRunning( false )
{
    auto cert = authenticator->sslCertificate();
    auto key  = authenticator->sslKey();
    auto dh   = authenticator->sslDhParameters();

    if ( !cert.empty() && !key.empty() && !dh.empty() )
    {
        CAFFA_INFO( "Loading SSL certificates" );

        m_sslContext = std::make_shared<ssl::context>( ssl::context::tlsv12 );

        m_sslContext->set_options( boost::asio::ssl::context::default_workarounds |
                                   boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use );

        m_sslContext->use_certificate_chain( boost::asio::buffer( cert.data(), cert.size() ) );

        m_sslContext->use_private_key( boost::asio::buffer( key.data(), key.size() ),
                                       boost::asio::ssl::context::file_format::pem );

        m_sslContext->use_tmp_dh( boost::asio::buffer( dh.data(), dh.size() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestServerApplication* RestServerApplication::instance()
{
    ServerApplication* appInstance = ServerApplication::instance();
    return dynamic_cast<RestServerApplication*>( appInstance );
}

int RestServerApplication::portNumber() const
{
    return m_portNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestServerApplication::run()
{
    RestServiceFactory::instance()->registerCreator<RestAppService>( "app" );
    RestServiceFactory::instance()->registerCreator<RestObjectService>( "objects" );
    RestServiceFactory::instance()->registerCreator<RestSessionService>( "sessions" );
    RestServiceFactory::instance()->registerCreator<RestDocumentService>( "documents" );
    RestServiceFactory::instance()->registerCreator<RestOpenApiService>( "openapi.json" );

    m_server = std::make_shared<RestServer>( m_ioContext,
                                             m_sslContext,
                                             tcp::endpoint{ net::ip::make_address( m_clientHost ), m_portNumber },
                                             ".",
                                             m_authenticator );

    onStartup();

    m_server->run();
    m_threadRunning = true;
    std::vector<thread> v;
    v.reserve( m_threads - 1 );
    for ( auto i = m_threads - 1; i > 0; --i )
        v.emplace_back( [this] { m_ioContext.run(); } );

    boost::asio::signal_set signals( m_ioContext, SIGINT, SIGTERM );
    signals.async_wait(
        [this]( const boost::system::error_code& error, int signal_number )
        {
            if ( error )
            {
                CAFFA_ERROR( "Quitting server due to signal " << error << ", signal: " << signal_number );
            }
            else
            {
                CAFFA_INFO( "Quitting server due to signal " << signal_number );
                quit();
            }
        } );

    m_ioContext.run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestServerApplication::quit()
{
    m_server->quit();
    m_ioContext.stop();
    onShutdown();
}

bool RestServerApplication::running() const
{
    return m_threadRunning && !m_ioContext.stopped();
}
