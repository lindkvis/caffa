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
#include "cafGrpcClientPassByRefObjectFactory.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcChildArrayFieldAccessor.h"
#include "cafGrpcChildFieldAccessor.h"
#include "cafGrpcClient.h"
#include "cafGrpcClientPassByValueObjectFactory.h"
#include "cafGrpcDataFieldAccessor.h"
#include "cafGrpcException.h"
#include "cafGrpcMethodAccessor.h"

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
    CAFFA_ASSERT( m_grpcClient );
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    CAFFA_TRACE( "Creating Passed-By-Reference Object of type " << classKeyword );

    auto objectHandle = caffa::DefaultObjectFactory::instance()->create( classKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( auto field : objectHandle->fields() )
    {
        if ( field->keyword() != "uuid" && field->capability<FieldScriptingCapability>() != nullptr )
        {
            applyAccessorToField( objectHandle.get(), field );
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
void ClientPassByRefObjectFactory::setGrpcClient( Client* client )
{
    m_grpcClient = client;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByRefObjectFactory::applyAccessorToField( caffa::ObjectHandle* fieldOwner, caffa::FieldHandle* fieldHandle )
{
    CAFFA_ASSERT( m_grpcClient );
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    if ( auto childField = dynamic_cast<caffa::ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<GrpcChildFieldAccessor>( m_grpcClient, childField ) );
    }
    else if ( auto childField = dynamic_cast<caffa::ChildArrayFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<GrpcChildArrayFieldAccessor>( m_grpcClient, childField ) );
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
            throw( Exception( grpc::Status( grpc::ABORTED,
                                            std::string( "Data type " ) + fieldHandle->dataType() +
                                                " not implemented in GRPC client" ) ) );
        }
        dataField->setUntypedAccessor( accessorCreator->create( m_grpcClient, fieldOwner, fieldHandle->keyword() ) );
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

void ClientPassByRefObjectFactory::applyAccessorToMethod( caffa::ObjectHandle* objectHandle,
                                                          caffa::MethodHandle* methodHandle )
{
    methodHandle->setAccessor( std::make_unique<MethodAccessor>( m_grpcClient,
                                                                 objectHandle,
                                                                 methodHandle,
                                                                 ClientPassByValueObjectFactory::instance() ) );
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
}

} // namespace caffa::rpc
