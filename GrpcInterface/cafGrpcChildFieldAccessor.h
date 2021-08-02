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

#include "cafChildFieldAccessor.h"
#include "cafGrpcClient.h"
#include "cafGrpcException.h"

namespace caffa::rpc
{
class GrpcChildFieldAccessor : public caffa::ChildFieldAccessor
{
public:
    GrpcChildFieldAccessor( Client* client, caffa::FieldHandle* fieldHandle )
        : caffa::ChildFieldAccessor( fieldHandle )
        , m_client( client )
    {
    }

    ObjectHandle* value() const override
    {
        m_cachedRemoteObject = m_client->getChildObject( m_field->ownerObject(), m_field->keyword() );
        return m_cachedRemoteObject.get();
    }

    void setValue( std::unique_ptr<ObjectHandle> value ) override { CAFFA_ASSERT( false && "Not implemented" ); }

    std::unique_ptr<ObjectHandle> remove( ObjectHandle* object )
    {
        CAFFA_ASSERT( false && "Not implemented" );
        return nullptr;
    }

private:
    Client* m_client;

    mutable std::unique_ptr<ObjectHandle> m_cachedRemoteObject;
};

} // namespace caffa::rpc
