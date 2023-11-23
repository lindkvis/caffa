// ##################################################################################################
//
//    Caffa
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
#include "cafRpcClientPassByRefObjectFactory.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafRpcChildArrayFieldAccessor.h"
#include "cafRpcChildFieldAccessor.h"
#include "cafRpcClient.h"
#include "cafRpcClientPassByValueObjectFactory.h"
#include "cafRpcDataFieldAccessor.h"
#include "cafRpcMethodAccessor.h"

#include <memory>

namespace caffa::rpc
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> ClientPassByRefObjectFactory::doCreate( const std::string_view& classKeyword )
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Rpc Client factory" );

    CAFFA_TRACE( "Creating Passed-By-Reference Object of type " << classKeyword );

    auto objectHandle = caffa::DefaultObjectFactory::instance()->create( classKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( auto field : objectHandle->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            if ( field->capability<FieldScriptingCapability>() != nullptr )
            {
                applyAccessorToField( objectHandle.get(), field );
            }
            else
            {
                applyNullAccessorToField( objectHandle.get(), field );
            }
        }
    }

    for ( auto method : objectHandle->methods() )
    {
        applyAccessorToMethod( objectHandle.get(), method );
    }

    return objectHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ClientPassByRefObjectFactory* ClientPassByRefObjectFactory::instance()
{
    static ClientPassByRefObjectFactory* fact = new ClientPassByRefObjectFactory;
    return fact;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::setClient( Client* client )
{
    m_client = client;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::applyAccessorToField( caffa::ObjectHandle* fieldOwner, caffa::FieldHandle* fieldHandle )
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

    if ( auto childField = dynamic_cast<caffa::ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<ChildFieldAccessor>( m_client, childField ) );
    }
    else if ( auto childField = dynamic_cast<caffa::ChildArrayFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<ChildArrayFieldAccessor>( m_client, childField ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::DataField*>( fieldHandle ); dataField )
    {
        CAFFA_TRACE( "Looking for an accessor creator for data type: " << fieldHandle->dataType() );
        AccessorCreatorBase* accessorCreator = nullptr;
        for ( auto& [dataType, storedAccessorCreator] : m_accessorCreatorMap )
        {
            CAFFA_TRACE( "Found one for " << dataType << " is that right?" );
            if ( dataType == fieldHandle->dataType() )
            {
                CAFFA_TRACE( "Yes!" );
                accessorCreator = storedAccessorCreator.get();
                break;
            }
        }
        if ( !accessorCreator )
        {
            throw std::runtime_error( std::string( "Data type " ) + fieldHandle->dataType() + " not implemented in client" );
        }
        dataField->setUntypedAccessor( accessorCreator->create( m_client, fieldHandle ) );
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::applyNullAccessorToField( caffa::ObjectHandle* fieldOwner,
                                                             caffa::FieldHandle*  fieldHandle )
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

    if ( auto childField = dynamic_cast<caffa::ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( nullptr );
    }
    else if ( auto childField = dynamic_cast<caffa::ChildArrayFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( nullptr );
    }
    else if ( auto dataField = dynamic_cast<caffa::DataField*>( fieldHandle ); dataField )
    {
        dataField->setUntypedAccessor( nullptr );
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

void ClientPassByRefObjectFactory::applyAccessorToMethod( caffa::ObjectHandle* objectHandle,
                                                          caffa::MethodHandle* methodHandle )
{
    CAFFA_TRACE( "Applying remote accessor to " << objectHandle->classKeyword() << "::" << methodHandle->keyword() << "()" );
    methodHandle->setAccessor(
        std::make_unique<MethodAccessor>( m_client, objectHandle, methodHandle, ClientPassByValueObjectFactory::instance() ) );
}

void ClientPassByRefObjectFactory::registerAccessorCreator( const std::string&                   dataType,
                                                            std::unique_ptr<AccessorCreatorBase> creator )
{
    CAFFA_TRACE( "Registering accessor for data type: " << dataType );
    m_accessorCreatorMap.insert( std::make_pair( dataType, std::move( creator ) ) );
}

void ClientPassByRefObjectFactory::registerAllBasicAccessorCreators()
{
    registerBasicAccessorCreators<double>();
    registerBasicAccessorCreators<float>();
    registerBasicAccessorCreators<int>();
    registerBasicAccessorCreators<int64_t>();
    registerBasicAccessorCreators<unsigned>();
    registerBasicAccessorCreators<uint64_t>();
    registerBasicAccessorCreators<bool>();
    registerBasicAccessorCreators<std::string>();
    registerBasicAccessorCreators<std::chrono::steady_clock::time_point>();
}

} // namespace caffa::rpc
