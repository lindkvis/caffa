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
class GrpcChildArrayFieldAccessor : public caffa::ChildArrayFieldAccessor
{
public:
    GrpcChildArrayFieldAccessor( Client* client, caffa::FieldHandle* fieldHandle )
        : caffa::ChildArrayFieldAccessor( fieldHandle )
        , m_client( client )
    {
    }

    size_t size() const override
    {
        m_remoteObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );
        return m_remoteObjects.size();
    }

    std::vector<std::unique_ptr<ObjectHandle>> clear() override
    {
        m_remoteObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );
        m_client->clearChildObjects( m_field->ownerObject(), m_field->keyword() );
        return std::move( m_remoteObjects );
    }

    std::vector<ObjectHandle*> value() const override
    {
        // TODO: We always overwrite the remote object here and it is mainly here to allow for
        // the same API on the client and server side and avoid the client code having to deal
        // with the memory. In the future we could time stamp and synchronise.
        m_remoteObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );

        std::vector<ObjectHandle*> rawPtrs;
        for ( auto& objectPtr : m_remoteObjects )
        {
            rawPtrs.push_back( objectPtr.get() );
        }
        return rawPtrs;
    }

    ObjectHandle* at( size_t index ) const
    {
        m_remoteObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );

        CAFFA_ASSERT( index < m_remoteObjects.size() );

        return m_remoteObjects[index].get();
    }

    void insert( size_t index, std::unique_ptr<ObjectHandle> pointer ) override
    {
        CAFFA_ASSERT( false && "Not implemented" );
    }
    void   push_back( std::unique_ptr<ObjectHandle> pointer ) { insert( size(), std::move( pointer ) ); }
    size_t index( const ObjectHandle* pointer ) const
    {
        m_remoteObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );
        for ( size_t i = 0; i < m_remoteObjects.size(); ++i )
        {
            if ( pointer == m_remoteObjects[i].get() )
            {
                return i;
            }
        }
        return -1;
    }
    std::unique_ptr<ObjectHandle> remove( size_t index )
    {
        CAFFA_ASSERT( index < size() );
        auto detachedPtr = std::move( m_remoteObjects[index] );
        m_remoteObjects.erase( m_remoteObjects.begin() + index );
        m_client->removeChildObject( m_field->ownerObject(), m_field->keyword(), index );
        return detachedPtr;
    }

private:
    Client* m_client;

    mutable std::vector<std::unique_ptr<ObjectHandle>> m_remoteObjects;
};

} // namespace caffa::rpc
