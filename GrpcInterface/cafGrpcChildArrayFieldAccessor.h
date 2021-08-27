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
        getRemoteObjectsIfNecessary();
        return m_remoteObjects.size();
    }

    std::vector<std::unique_ptr<ObjectHandle>> clear() override
    {
        getRemoteObjectsIfNecessary();
        m_client->clearChildObjects( m_field->ownerObject(), m_field->keyword() );

        std::vector<std::unique_ptr<ObjectHandle>> removedObjects;
        removedObjects.swap( m_remoteObjects );
        return removedObjects;
    }

    std::vector<ObjectHandle*> value() const override
    {
        getRemoteObjectsIfNecessary();

        std::vector<ObjectHandle*> rawPtrs;
        for ( auto& objectPtr : m_remoteObjects )
        {
            rawPtrs.push_back( objectPtr.get() );
        }
        return rawPtrs;
    }

    ObjectHandle* at( size_t index ) const
    {
        getRemoteObjectsIfNecessary();

        CAFFA_ASSERT( index < m_remoteObjects.size() );

        return m_remoteObjects[index].get();
    }

    void insert( size_t index, std::unique_ptr<ObjectHandle> pointer ) override
    {
        auto   object  = std::move( pointer );
        size_t oldSize = m_remoteObjects.size();
        m_client->insertChildObject( m_field->ownerObject(), m_field->keyword(), index, object.get() );
        m_remoteObjects.insert( m_remoteObjects.begin() + index, std::move( pointer ) );
        CAFFA_ASSERT( m_remoteObjects.size() == ( oldSize + 1u ) );
    }

    void push_back( std::unique_ptr<ObjectHandle> pointer ) { insert( size(), std::move( pointer ) ); }

    size_t index( const ObjectHandle* pointer ) const
    {
        getRemoteObjectsIfNecessary();
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
    // TODO: This needs to be more sophisticated. At the moment we get the remote objects no matter what.
    void getRemoteObjectsIfNecessary() const
    {
        auto serverObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );
        if ( m_remoteObjects.size() != serverObjects.size() )
        {
            m_remoteObjects.swap( serverObjects );
        }
    }

private:
    Client* m_client;

    mutable std::vector<std::unique_ptr<ObjectHandle>> m_remoteObjects;
};

} // namespace caffa::rpc
