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
#include "cafChildFieldAccessor.h"

#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ChildFieldDirectStorageAccessor::ChildFieldDirectStorageAccessor( FieldHandle* field )
    : ChildFieldAccessor( field )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ChildFieldDirectStorageAccessor::object()
{
    return m_object.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const ObjectHandle* ChildFieldDirectStorageAccessor::object() const
{
    return m_object.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ChildFieldDirectStorageAccessor::setObject( std::unique_ptr<ObjectHandle> object )
{
    if ( m_object )
    {
        m_object->disconnectObserverFromAllSignals( m_field->ownerObject() );
    }
    m_object = std::move( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ChildFieldDirectStorageAccessor::deepCloneObject() const
{
    if ( !m_object ) return nullptr;

    JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
    return serializer.copyBySerialization( m_object.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ChildFieldDirectStorageAccessor::deepCopyObjectFrom( const ObjectHandle* copyFrom )
{
    JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
    if ( !m_object )
    {
        m_object = serializer.copyBySerialization( copyFrom );
    }
    else
    {
        std::string json = serializer.writeObjectToString( copyFrom );
        serializer.readObjectFromString( m_object.get(), json );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ChildFieldDirectStorageAccessor::clear()
{
    m_object->disconnectObserverFromAllSignals( m_field->ownerObject() );
    return std::move( m_object );
}