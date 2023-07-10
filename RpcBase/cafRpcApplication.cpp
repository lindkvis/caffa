// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- 3d Radar AS
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
#include "cafRpcApplication.h"

#include "cafLogger.h"

#include <fstream>
#include <sstream>

using namespace caffa::rpc;

RpcApplication::RpcApplication( unsigned int capabilities )
    : caffa::Application( capabilities )
{
}

RpcApplication::RpcApplication( AppInfo::AppCapability capability )
    : caffa::Application( capability )
{
}

RpcApplication* RpcApplication::instance()
{
    caffa::Application* appInstance = caffa::Application::instance();
    return dynamic_cast<RpcApplication*>( appInstance );
}

std::string RpcApplication::readKeyAndCertificate( const std::string& path )
{
    std::ifstream stream( path );
    if ( !stream.good() )
    {
        CAFFA_CRITICAL( "Failed to open file: " << path );
    }
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}
