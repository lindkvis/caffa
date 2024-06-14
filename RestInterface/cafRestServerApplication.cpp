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

#include "cafAssert.h"
#include "cafLogger.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestServerApplication::RestServerApplication( const std::string&                       clientHost,
                                              unsigned short                           portNumber,
                                              int                                      threads,
                                              std::shared_ptr<const RestAuthenticator> authenticator )
    : ServerApplication( AppInfo::AppCapability::SERVER )
    , m_portNumber( portNumber )
    , m_threads( threads )
    , m_authenticator( authenticator )
    , m_threadRunning( false )
{
    auto cert = authenticator->sslCertificate();
    auto key  = authenticator->sslKey();
    auto dh   = authenticator->sslDhParameters();

    if ( !cert.empty() && !key.empty() && !dh.empty() )
    {
        CAFFA_INFO( "Loading SSL certificates" );
    }

    m_server = std::make_shared<RestServer>( clientHost, m_threads, m_portNumber, m_authenticator );
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
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestAppService>( "app" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestObjectService>( "objects" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestSessionService>( "sessions" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestDocumentService>( "documents" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestOpenApiService>( "openapi.json" );

    onStartup();

    m_threadRunning = true;
    m_server->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RestServerApplication::quit()
{
    m_server->quit();
    onShutdown();
}

bool RestServerApplication::running() const
{
    return m_threadRunning;
}

void RestServerApplication::waitUntilReady() const
{
    m_server->waitUntilReady();
}
