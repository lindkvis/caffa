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
std::shared_ptr<ObjectHandle> ClientPassByValueObjectFactory::doCreate( const std::string_view& classKeyword )
{
    CAFFA_ASSERT( m_client );
    if ( !m_client ) throw std::runtime_error( "No Client set in Client factory" );

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
std::shared_ptr<ClientPassByValueObjectFactory> ClientPassByValueObjectFactory::instance()
{
    static auto objectFactory = std::shared_ptr<ClientPassByValueObjectFactory>( new ClientPassByValueObjectFactory );
    return objectFactory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ClientPassByValueObjectFactory::setClient( Client* client )
{
    m_client = client;
}

void ClientPassByValueObjectFactory::applyAccessorToMethod( caffa::ObjectHandle* objectHandle,
                                                            caffa::MethodHandle* methodHandle )
{
    methodHandle->setAccessor(
        std::make_unique<MethodAccessor>( m_client, objectHandle, methodHandle, shared_from_this() ) );
}

} // namespace caffa::rpc
