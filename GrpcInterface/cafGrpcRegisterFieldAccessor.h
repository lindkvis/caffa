//##################################################################################################
//
//   Caffa
//   Copyright (C) 3d-Radar AS
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
#pragma once

#include "cafGrpcClient.h"
#include "cafGrpcException.h"
#include "cafRegisterField.h"

namespace caffa::rpc
{
class GrpcRegisterFieldAccessor : public caffa::RegisterFieldAccessorInterface
{
public:
    GrpcRegisterFieldAccessor( Client* client, caffa::ObjectHandle* fieldOwner, const std::string& fieldName )
        : m_client( client )
        , m_fieldOwner( fieldOwner )
        , m_fieldName( fieldName )
    {
    }

    uint32_t read( uint32_t addressOffset ) const override
    {
        //        if ( !m_client ) throw Exception( Exception::Type::BadConnection, "No Client set in accessor" );
        // if ( !m_fieldOwner )
        // throw Exception( Exception::Type::BadConfiguration, "No field owner for rpc field accessor" );
        return m_client->get<uint32_t>( m_fieldOwner, m_fieldName, addressOffset );
    }

    void write( uint32_t addressOffset, uint32_t value ) override
    {
        //        if ( !m_client ) throw Exception( Exception::Type::BadConnection, "No Client set in accessor" );
        // if ( !m_fieldOwner )
        //  throw Exception( Exception::Type::BadConfiguration, "No field owner for rpc field accessor" );
        return m_client->set<uint32_t>( m_fieldOwner, m_fieldName, value, addressOffset );
    }

private:
    Client* m_client;

    caffa::ObjectHandle* m_fieldOwner;
    std::string          m_fieldName;
};

} // namespace caffa::rpc
