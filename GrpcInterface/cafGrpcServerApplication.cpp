// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- Kontur AS
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
#include "cafGrpcServerApplication.h"

#include "cafAssert.h"
#include "cafGrpcAppService.h"
#include "cafGrpcFieldService.h"
#include "cafGrpcObjectService.h"
#include "cafGrpcServer.h"
#include "cafGrpcServiceInterface.h"
#include "cafLogger.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcServerApplication::GrpcServerApplication( int                portNumber,
                                              const std::string& serverCertFile /* = ""*/,
                                              const std::string& serverKeyFile /* = ""*/,
                                              const std::string& caCertFile /* = ""*/ )
    : ServerApplication( AppInfo::AppCapability::SERVER )

{
    m_server = std::make_unique<caffa::rpc::GrpcServer>( portNumber, serverCertFile, serverKeyFile, caCertFile );

    caffa::rpc::ServiceFactory::instance()->registerCreator<caffa::rpc::GrpcAppService>(
        typeid( caffa::rpc::GrpcAppService ).hash_code() );
    caffa::rpc::ServiceFactory::instance()->registerCreator<caffa::rpc::GrpcFieldService>(
        typeid( caffa::rpc::GrpcFieldService ).hash_code() );
    caffa::rpc::ServiceFactory::instance()->registerCreator<caffa::rpc::GrpcObjectService>(
        typeid( caffa::rpc::GrpcObjectService ).hash_code() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcServerApplication* caffa::rpc::GrpcServerApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<GrpcServerApplication*>( appInstance );
}

int GrpcServerApplication::portNumber() const
{
    return m_server->port();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::rpc::GrpcServerApplication::run()
{
    CAFFA_ASSERT( m_server );
    onStartup();
    m_server->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcServerApplication::quit()
{
    m_server->quit();
    onShutdown();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool GrpcServerApplication::running() const
{
    return m_server->running();
}
