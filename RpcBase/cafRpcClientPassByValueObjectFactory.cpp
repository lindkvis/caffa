// ##################################################################################################
//
//    Caffa - File copied and altered from cafClientPassByRefObjectFactory.cpp
//
//    Copyright (C) 2011- Ceetron AS (Changes up until April 2021)
//    Copyright (C) 2021- Kontur AS (Changes from April 2021 and onwards)
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

#include "cafRpcClientPassByValueObjectFactory.h"

#include "cafDefaultObjectFactory.h"
#include "cafField.h"
#include "cafRpcChildArrayFieldAccessor.h"
#include "cafRpcClient.h"
#include "cafRpcDataFieldAccessor.h"
#include "cafRpcMethodAccessor.h"

namespace caffa::rpc
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> ClientPassByValueObjectFactory::doCreate( const std::string_view& classKeyword )
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

    CAFFA_TRACE( "Creating Passed-By-Value Object of type " << classKeyword );

    auto objectHandle = DefaultObjectFactory::instance()->create( classKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( const auto field : objectHandle->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            applyAccessorToField( field );
        }
    }

    for ( const auto method : objectHandle->methods() )
    {
        applyAccessorToMethod( objectHandle.get(), method );
    }

    return objectHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ClientPassByValueObjectFactory> ClientPassByValueObjectFactory::instance()
{
    static auto fact = std::shared_ptr<ClientPassByValueObjectFactory>( new ClientPassByValueObjectFactory );
    return fact;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByValueObjectFactory::setClient( Client* client )
{
    m_client = client;
}

void ClientPassByValueObjectFactory::applyAccessorToField( FieldHandle* fieldHandle ) const
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

    if ( auto childField = dynamic_cast<ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<ChildFieldDirectStorageAccessor>( childField ) );
    }
    else if ( auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( fieldHandle ); childArrayField )
    {
        childArrayField->setAccessor( std::make_unique<ChildArrayFieldDirectStorageAccessor>( childArrayField ) );
    }
    else if ( auto dataField = dynamic_cast<DataField*>( fieldHandle ); dataField )
    {
        if ( auto jsonCapability = fieldHandle->capability<FieldIoCapability>(); jsonCapability )
        {
            dataField->applyDirectStorageAccessor();
        }
        else
        {
            CAFFA_ASSERT( false && "All fields that are scriptable has to be serializable" );
            throw std::runtime_error( "Field " + fieldHandle->keyword() + " is not serializable" );
        }
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

void ClientPassByValueObjectFactory::applyAccessorToMethod( ObjectHandle* objectHandle, MethodHandle* methodHandle )
{
    methodHandle->setAccessor( std::make_unique<MethodAccessor>( m_client, objectHandle, methodHandle, this ) );
}

} // namespace caffa::rpc
