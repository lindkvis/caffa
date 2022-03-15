//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar AS
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
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
ServerApplication::ServerApplication( int                portNumber,
                                      const std::string& serverCertFile /* = ""*/,
                                      const std::string& serverKeyFile /* = ""*/,
                                      const std::string& caCertFile /* = ""*/ )
    : Application( AppInfo::AppCapability::GRPC_SERVER )

{
    m_server = std::make_unique<caffa::rpc::Server>( portNumber, serverCertFile, serverKeyFile, caCertFile );

    caffa::rpc::ServiceFactory::instance()->registerCreator<caffa::rpc::AppService>(
        typeid( caffa::rpc::AppService ).hash_code() );
    caffa::rpc::ServiceFactory::instance()->registerCreator<caffa::rpc::FieldService>(
        typeid( caffa::rpc::FieldService ).hash_code() );
    caffa::rpc::ServiceFactory::instance()->registerCreator<caffa::rpc::ObjectService>(
        typeid( caffa::rpc::ObjectService ).hash_code() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ServerApplication* caffa::rpc::ServerApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<ServerApplication*>( appInstance );
}

int ServerApplication::portNumber() const
{
    return m_server->port();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::rpc::ServerApplication::run()
{
    CAFFA_ASSERT( m_server );
    caffa::Logger::registerThreadName( "serverapp_thread" );
    onStartup();
    m_server->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ServerApplication::quit()
{
    m_server->quit();
    onShutdown();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ServerApplication::running() const
{
    return m_server->running();
}
