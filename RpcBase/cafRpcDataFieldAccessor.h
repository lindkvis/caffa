// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
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

#include "cafDataFieldAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafRpcClient.h"

namespace caffa::rpc
{
template <class DataType>
class DataFieldAccessor : public caffa::DataFieldAccessor<DataType>
{
public:
    DataFieldAccessor( Client* client, caffa::FieldHandle* fieldHandle )
        : caffa::DataFieldAccessor<DataType>()
        , m_client( client )
        , m_fieldHandle( fieldHandle )
    {
    }

    std::unique_ptr<caffa::DataFieldAccessor<DataType>> clone() const override
    {
        return std::make_unique<DataFieldAccessor<DataType>>( m_client, m_fieldHandle );
    }

    DataType value() override
    {
        return m_client->get<DataType>( m_fieldHandle->ownerObject(), m_fieldHandle->keyword() );
    }

    void setValue( const DataType& value ) override
    {
        return m_client->set<DataType>( m_fieldHandle->ownerObject(), m_fieldHandle->keyword(), value );
    }

    bool hasGetter() const override
    {
        auto scriptability = m_fieldHandle->capability<caffa::FieldScriptingCapability>();
        return scriptability && scriptability->isReadable();
    }
    bool hasSetter() const override
    {
        auto scriptability = m_fieldHandle->capability<caffa::FieldScriptingCapability>();
        return scriptability && scriptability->isWritable();
    }

private:
    Client* m_client;

    caffa::FieldHandle* m_fieldHandle;
};

} // namespace caffa::rpc
