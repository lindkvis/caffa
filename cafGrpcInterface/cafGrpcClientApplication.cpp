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
#include "cafGrpcClientApplication.h"
#include "cafGrpcClientObjectFactory.h"

#include "cafAssert.h"
#include "cafGrpcClient.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ClientApplication::ClientApplication( const std::string& hostname, int portNumber )
    : Application( { AppCapability::GRPC_CLIENT } )

{
    m_client = std::make_unique<caffa::rpc::Client>( hostname, portNumber );
    caffa::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( m_client.get());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ClientApplication* ClientApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<ClientApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Client* ClientApplication::client() 
{
    return m_client.get();    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Client* ClientApplication::client() const
{
    return m_client.get();    
}
