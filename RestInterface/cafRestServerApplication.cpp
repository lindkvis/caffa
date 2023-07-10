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
#include "cafRestObjectService.h"
#include "cafRestSchemaService.h"
#include "cafRestServer.h"
#include "cafRestServiceInterface.h"
#include "cafRestSessionService.h"

#include "cafAssert.h"
#include "cafLogger.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestServerApplication::RestServerApplication( unsigned short portNumber, int threads )
    : ServerApplication( AppInfo::AppCapability::SERVER )
    , m_portNumber( portNumber )
    , m_threads( threads )
    , m_ioContext( threads )
{
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestAppService>( "app" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestObjectService>( "object" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestSchemaService>( "schemas" );
    caffa::rpc::RestServiceFactory::instance()->registerCreator<caffa::rpc::RestSessionService>( "session" );

    m_server =
        std::make_shared<RestServer>( m_ioContext, tcp::endpoint{ net::ip::make_address( "0.0.0.0" ), m_portNumber }, "." );
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
    CAFFA_ASSERT( m_server );
    onStartup();

    m_server->run();
    std::vector<std::thread> v;
    v.reserve( m_threads - 1 );
    for ( auto i = m_threads - 1; i > 0; --i )
        v.emplace_back( [this] { m_ioContext.run(); } );
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
    return !m_ioContext.stopped();
}