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
// ##################################################################################################
#pragma once

#include "cafMethodHandle.h"
#include "cafRpcClient.h"

namespace caffa::rpc
{
class MethodAccessor : public caffa::MethodAccessorInterface
{
public:
    MethodAccessor( const Client*              client,
                    const caffa::ObjectHandle* selfHandle,
                    caffa::MethodHandle*       methodHandle,
                    caffa::ObjectFactory*      objectFactory )
        : MethodAccessorInterface( selfHandle, methodHandle, objectFactory )
        , m_client( client )
    {
    }

    std::string execute( const std::string& jsonArguments ) const
    {
        return m_client->execute( m_selfHandle, m_methodHandle->keyword(), jsonArguments );
    }

private:
    const Client* m_client;
};
} // namespace caffa::rpc