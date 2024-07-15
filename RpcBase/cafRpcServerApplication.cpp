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
#include "cafRpcServerApplication.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ServerApplication::ServerApplication( unsigned capability )
    : RpcApplication( capability )
    , m_requiresValidSession( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ServerApplication::ServerApplication( AppInfo::AppCapability capability )
    : RpcApplication( capability )
    , m_requiresValidSession( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ServerApplication* caffa::rpc::ServerApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<ServerApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::rpc::ServerApplication::requiresValidSession() const
{
    return m_requiresValidSession;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::rpc::ServerApplication::setRequiresValidSession( bool requiresValidSession )
{
    m_requiresValidSession = requiresValidSession;
}
