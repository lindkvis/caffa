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
#include "cafJsonSerializer.h"

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
        m_remoteObject = getShallowCopyOfRemoteObject();
        return m_remoteObject.get();
    }

    void setValue( std::unique_ptr<ObjectHandle> value ) override
    {
        m_remoteObject = std::move( value );
        m_client->setChildObject( m_field->ownerObject(), m_field->keyword(), m_remoteObject.get() );
    }

    std::unique_ptr<ObjectHandle> clear() override
    {
        m_remoteObject = getShallowCopyOfRemoteObject();
        m_client->clearChildObjects( m_field->ownerObject(), m_field->keyword() );
        return std::move( m_remoteObject );
    }

    std::unique_ptr<ObjectHandle> cloneValue() const override { return getDeepCopyOfRemoteObject(); }

    void copyValue( const ObjectHandle* copyFrom )
    {
        if ( m_remoteObject )
        {
            JsonSerializer serializer;
            std::string    json = serializer.writeObjectToString( copyFrom );
            serializer.readObjectFromString( m_remoteObject.get(), json );
        }
        CAFFA_INFO( "Trying to copy object back to server" );
        m_client->deepCopyChildObject( m_field->ownerObject(), m_field->keyword(), copyFrom );
    }

private:
    std::unique_ptr<ObjectHandle> getShallowCopyOfRemoteObject() const
    {
        return m_client->getShallowCopyOfChildObject( m_field->ownerObject(), m_field->keyword() );
    }

    std::unique_ptr<ObjectHandle> getDeepCopyOfRemoteObject() const
    {
        return m_client->getDeepCopyOfChildObject( m_field->ownerObject(), m_field->keyword() );
    }

private:
    Client* m_client;

    mutable std::unique_ptr<ObjectHandle> m_remoteObject;
};

} // namespace caffa::rpc
