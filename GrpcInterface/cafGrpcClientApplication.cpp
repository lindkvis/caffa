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
#include "cafGrpcClientApplication.h"

#include "cafAssert.h"
#include "cafGrpcClient.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcClientApplication::GrpcClientApplication( const std::string& hostname, int portNumber )
    : RpcApplication( { AppInfo::AppCapability::CLIENT } )

{
    m_client = std::make_unique<caffa::rpc::GrpcClient>( caffa::Session::Type::REGULAR, hostname, portNumber );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcClientApplication* GrpcClientApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<GrpcClientApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcClient* GrpcClientApplication::client()
{
    return m_client.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const GrpcClient* GrpcClientApplication::client() const
{
    return m_client.get();
}
