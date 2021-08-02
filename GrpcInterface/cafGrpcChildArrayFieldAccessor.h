//##################################################################################################
//
//   Caffa
//   Copyright (C) 3D-Radar AS
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

#include "cafChildArrayFieldAccessor.h"
#include "cafGrpcClient.h"
#include "cafGrpcException.h"

namespace caffa::rpc
{
template <class DataType>
class GrpcChildArrayFieldAccessor : public caffa::ChildArrayFieldAccessor<DataType>
{
public:
    GrpcChildArrayFieldAccessor( Client* client, caffa::ObjectHandle* fieldOwner, const std::string& fieldName )
        : caffa::ChildArrayFieldAccessor<DataType>()
        , m_client( client )
        , m_fieldOwner( fieldOwner )
        , m_fieldName( fieldName )
    {
    }

    DataType* value() override
    {
        CAFFA_ASSERT( false && "Not implemented" );
        return nullptr;
    }

    void setValue( std::unique_ptr<DataType> value ) override { CAFFA_ASSERT( false && "Not implemented" ); }

    std::unique_ptr<DataType> remove( ObjectHandle* object )
    {
        CAFFA_ASSERT( false && "Not implemented" );
        return nullptr;
    }

private:
    Client* m_client;

    caffa::ObjectHandle* m_fieldOwner;
    std::string          m_fieldName;
};

} // namespace caffa::rpc
