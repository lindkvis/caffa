// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
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
#include "cafRestClientApplication.h"

#include "cafAssert.h"
#include "cafRestClient.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClientApplication::RestClientApplication( const std::string& hostname, int portNumber )
    : RpcApplication( { AppInfo::AppCapability::CLIENT } )
{
    m_client = std::make_unique<caffa::rpc::RestClient>( caffa::Session::Type::REGULAR, hostname, portNumber );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClientApplication* RestClientApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<RestClientApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestClient* RestClientApplication::client()
{
    return m_client.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RestClient* RestClientApplication::client() const
{
    return m_client.get();
}
