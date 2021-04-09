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

using namespace caf::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ServerApplication::ServerApplication( int portNumber )
    : Application( AppCapability::GRPC_SERVER )

{
    m_server = std::make_unique<caf::rpc::Server>( portNumber );

    caf::rpc::ServiceFactory::instance()->registerCreator<caf::rpc::AppService>(
        typeid( caf::rpc::AppService ).hash_code() );
    caf::rpc::ServiceFactory::instance()->registerCreator<caf::rpc::FieldService>(
        typeid( caf::rpc::FieldService ).hash_code() );
    caf::rpc::ServiceFactory::instance()->registerCreator<caf::rpc::ObjectService>(
        typeid( caf::rpc::ObjectService ).hash_code() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ServerApplication* ServerApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<ServerApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ServerApplication::run()
{
    CAF_ASSERT( m_server );
    m_server->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ServerApplication::quit()
{
    m_server->quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ServerApplication::running() const
{
    return m_server->running();
}
