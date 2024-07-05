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

#include "cafChildFieldAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafRpcClient.h"

namespace caffa::rpc
{
class ChildFieldAccessor : public caffa::ChildFieldAccessor
{
public:
    ChildFieldAccessor( Client* client, caffa::FieldHandle* fieldHandle )
        : caffa::ChildFieldAccessor( fieldHandle )
        , m_client( client )
    {
    }

    std::shared_ptr<ObjectHandle> object() override
    {
        m_remoteObject = getShallowCopyOfRemoteObject();
        return m_remoteObject;
    }

    std::shared_ptr<const ObjectHandle> object() const override
    {
        m_remoteObject = getShallowCopyOfRemoteObject();
        return m_remoteObject;
    }

    void setObject( std::shared_ptr<ObjectHandle> object ) override
    {
        m_remoteObject = object;
        m_client->setChildObject( m_field->ownerObject(), m_field->keyword(), m_remoteObject.get() );
    }

    void clear() override
    {
        m_remoteObject.reset();
        m_client->clearChildObjects( m_field->ownerObject(), m_field->keyword() );
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
    std::shared_ptr<ObjectHandle> getShallowCopyOfRemoteObject() const
    {
        return m_client->getChildObject( m_field->ownerObject(), m_field->keyword() );
    }

private:
    Client* m_client;

    mutable std::shared_ptr<ObjectHandle> m_remoteObject;
};

} // namespace caffa::rpc
