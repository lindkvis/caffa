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
#include "cafRpcMethodAccessor.h"

#include <memory>
#include <ranges>

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

    auto objectHandle = DefaultObjectFactory::instance()->create( classKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( const auto field : objectHandle->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            if ( field->capability<FieldScriptingCapability>() != nullptr )
            {
                applyAccessorToField( field );
            }
            else
            {
                applyNullAccessorToField( field );
            }
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
std::shared_ptr<ClientPassByRefObjectFactory> ClientPassByRefObjectFactory::instance()
{
    static auto fact = std::shared_ptr<ClientPassByRefObjectFactory>( new ClientPassByRefObjectFactory );
    return fact;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::setClient( Client* client )
{
    m_client = client;
}

std::list<std::string> ClientPassByRefObjectFactory::supportedDataTypes() const
{
    std::list<std::string> dataTypes;

    for ( const auto& creator : std::views::values( m_accessorCreatorMap ) )
    {
        dataTypes.push_back( creator->jsonDataType() );
    }

    return dataTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::applyAccessorToField( FieldHandle* fieldHandle ) const
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

    if ( auto childField = dynamic_cast<ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<ChildFieldAccessor>( m_client, childField ) );
    }
    else if ( auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( fieldHandle ); childArrayField )
    {
        childArrayField->setAccessor( std::make_unique<ChildArrayFieldAccessor>( m_client, childArrayField ) );
    }
    else if ( auto dataField = dynamic_cast<DataField*>( fieldHandle ); dataField )
    {
        if ( auto jsonCapability = fieldHandle->capability<FieldIoCapability>(); jsonCapability )
        {
            AccessorCreatorBase* accessorCreator = nullptr;
            auto                 fieldDataType   = fieldHandle->dataType();

            CAFFA_TRACE( "Looking for an accessor creator for data type: " << fieldDataType );

            if ( const auto it = m_accessorCreatorMap.find( fieldDataType ); it != m_accessorCreatorMap.end() )
            {
                accessorCreator = it->second.get();
            }
            if ( !accessorCreator )
            {
                throw std::runtime_error( std::string( "Data type " ) + fieldHandle->dataType() +
                                          " not implemented in client" );
            }
            CAFFA_ASSERT( accessorCreator );
            auto accessor = accessorCreator->create( m_client, fieldHandle );
            CAFFA_ASSERT( accessor );

            dataField->setUntypedAccessor( std::move( accessor ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::applyNullAccessorToField( FieldHandle* fieldHandle ) const
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

    if ( auto childField = dynamic_cast<ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( nullptr );
    }
    else if ( auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( fieldHandle ); childArrayField )
    {
        childArrayField->setAccessor( nullptr );
    }
    else if ( auto dataField = dynamic_cast<DataField*>( fieldHandle ); dataField )
    {
        dataField->setUntypedAccessor( nullptr );
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

void ClientPassByRefObjectFactory::applyAccessorToMethod( ObjectHandle* objectHandle, MethodHandle* methodHandle ) const
{
    CAFFA_TRACE( "Applying remote accessor to " << objectHandle->classKeyword() << "::" << methodHandle->keyword() << "()" );
    methodHandle->setAccessor( std::make_unique<MethodAccessor>( m_client,
                                                                 objectHandle,
                                                                 methodHandle,
                                                                 ClientPassByValueObjectFactory::instance().get() ) );
}

void ClientPassByRefObjectFactory::registerAccessorCreator( const std::string&                   dataType,
                                                            std::unique_ptr<AccessorCreatorBase> creator )
{
    CAFFA_TRACE( "Registering accessor for data type: " << dataType << ", -> " << creator->jsonDataType() );
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
    registerBasicAccessorCreators<std::chrono::system_clock::time_point>();
    registerBasicAccessorCreators<std::chrono::nanoseconds>();
    registerBasicAccessorCreators<std::chrono::microseconds>();
    registerBasicAccessorCreators<std::chrono::milliseconds>();
    registerBasicAccessorCreators<std::chrono::seconds>();
}

} // namespace caffa::rpc
