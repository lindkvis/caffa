// ##################################################################################################
//
//    Caffa
//    Copyright (C) 3D-Radar AS
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
#pragma once

#include "cafChildArrayFieldAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafRpcClient.h"

#include <limits>

namespace caffa::rpc
{
class ChildArrayFieldAccessor : public caffa::ChildArrayFieldAccessor
{
public:
    ChildArrayFieldAccessor( Client* client, caffa::FieldHandle* fieldHandle )
        : caffa::ChildArrayFieldAccessor( fieldHandle )
        , m_client( client )
    {
    }

    size_t size() const override
    {
        getRemoteObjectsIfNecessary();
        return m_remoteObjects.size();
    }

    void clear() override
    {
        m_client->clearChildObjects( m_field->ownerObject(), m_field->keyword() );

        std::vector<std::shared_ptr<ObjectHandle>> removedObjects;
        removedObjects.swap( m_remoteObjects );
    }

    std::vector<std::shared_ptr<ObjectHandle>> objects() override
    {
        getRemoteObjectsIfNecessary();
        return m_remoteObjects;
    }

    std::vector<std::shared_ptr<const ObjectHandle>> objects() const override
    {
        getRemoteObjectsIfNecessary();

        std::vector<std::shared_ptr<const ObjectHandle>> constPtrs;
        for ( auto objectPtr : m_remoteObjects )
        {
            constPtrs.push_back( objectPtr );
        }
        return constPtrs;
    }

    std::shared_ptr<ObjectHandle> at( size_t index ) const override
    {
        getRemoteObjectsIfNecessary();

        CAFFA_ASSERT( index < m_remoteObjects.size() );

        return m_remoteObjects[index];
    }

    void insert( size_t index, std::shared_ptr<ObjectHandle> pointer ) override
    {
        size_t oldSize = m_remoteObjects.size();
        m_client->insertChildObject( m_field->ownerObject(), m_field->keyword(), index, pointer.get() );
        m_remoteObjects.insert( m_remoteObjects.begin() + index, pointer );
        CAFFA_ASSERT( m_remoteObjects.size() == ( oldSize + 1u ) );
    }

    void push_back( std::shared_ptr<ObjectHandle> pointer ) override { insert( size(), pointer ); }

    size_t index( std::shared_ptr<const ObjectHandle> pointer ) const override
    {
        getRemoteObjectsIfNecessary();
        for ( size_t i = 0; i < m_remoteObjects.size(); ++i )
        {
            if ( pointer.get() == m_remoteObjects[i].get() )
            {
                return i;
            }
        }
        return std::numeric_limits<size_t>::max();
    }
    void remove( size_t index ) override
    {
        CAFFA_ASSERT( index < size() );
        m_remoteObjects.erase( m_remoteObjects.begin() + index );
        m_client->removeChildObject( m_field->ownerObject(), m_field->keyword(), index );
    }

    bool hasGetter() const override
    {
        auto scriptability = m_field->capability<caffa::FieldScriptingCapability>();
        return scriptability && scriptability->isReadable();
    }

    bool hasSetter() const override
    {
        auto scriptability = m_field->capability<caffa::FieldScriptingCapability>();
        return scriptability && scriptability->isWritable();
    }

private:
    // TODO: This needs to be more sophisticated. At the moment we get the remote objects no matter what.
    void getRemoteObjectsIfNecessary() const
    {
        m_remoteObjects = m_client->getChildObjects( m_field->ownerObject(), m_field->keyword() );
    }

private:
    Client* m_client;

    mutable std::vector<std::shared_ptr<ObjectHandle>> m_remoteObjects;
};

} // namespace caffa::rpc
