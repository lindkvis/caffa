// ##################################################################################################
//
//    Caffa - File copied and altered from cafGrpcClientPassByRefObjectFactory.cpp
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

#include "cafGrpcClientPassByValueObjectFactory.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcChildArrayFieldAccessor.h"
#include "cafGrpcChildFieldAccessor.h"
#include "cafGrpcClient.h"
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
std::shared_ptr<ObjectHandle> ClientPassByValueObjectFactory::doCreate( const std::string_view& classKeyword )
{
    CAFFA_ASSERT( m_grpcClient );
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    CAFFA_TRACE( "Creating Passed-By-Value Object of type " << classKeyword );

    auto objectHandle = caffa::DefaultObjectFactory::instance()->create( classKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( auto method : objectHandle->methods() )
    {
        applyAccessorToMethod( objectHandle.get(), method );
    }

    return objectHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ClientPassByValueObjectFactory* ClientPassByValueObjectFactory::instance()
{
    static ClientPassByValueObjectFactory* fact = new ClientPassByValueObjectFactory;
    return fact;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByValueObjectFactory::setGrpcClient( Client* client )
{
    m_grpcClient = client;
}

void ClientPassByValueObjectFactory::applyAccessorToMethod( caffa::ObjectHandle* objectHandle,
                                                            caffa::MethodHandle* methodHandle )
{
    methodHandle->setAccessor( std::make_unique<MethodAccessor>( m_grpcClient, objectHandle, methodHandle, this ) );
}

} // namespace caffa::rpc
