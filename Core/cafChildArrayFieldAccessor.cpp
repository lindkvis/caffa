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
#include "cafChildArrayFieldAccessor.h"

#include "cafFieldHandle.h"
#include "cafObjectHandle.h"

using namespace caffa;

ChildArrayFieldDirectStorageAccessor::ChildArrayFieldDirectStorageAccessor( FieldHandle* field )
    : ChildArrayFieldAccessor( field )
{
}
ChildArrayFieldDirectStorageAccessor::~ChildArrayFieldDirectStorageAccessor()
{
}

size_t ChildArrayFieldDirectStorageAccessor::size() const
{
    return m_pointers.size();
}
std::vector<std::unique_ptr<ObjectHandle>> ChildArrayFieldDirectStorageAccessor::clear()
{
    for ( auto& ptr : m_pointers )
    {
        ptr->disconnectObserverFromAllSignals( m_field->ownerObject() );
    }

    std::vector<std::unique_ptr<ObjectHandle>> returnValues;
    returnValues.swap( m_pointers );

    return returnValues;
}

std::vector<ObjectHandle*> ChildArrayFieldDirectStorageAccessor::objects()
{
    std::vector<ObjectHandle*> rawPointers;
    for ( auto& ptr : m_pointers )
    {
        rawPointers.push_back( ptr.get() );
    }
    return rawPointers;
}

std::vector<const ObjectHandle*> ChildArrayFieldDirectStorageAccessor::objects() const
{
    std::vector<const ObjectHandle*> rawPointers;
    for ( auto& ptr : m_pointers )
    {
        rawPointers.push_back( ptr.get() );
    }
    return rawPointers;
}

ObjectHandle* ChildArrayFieldDirectStorageAccessor::at( size_t index ) const
{
    CAFFA_ASSERT( index < m_pointers.size() );
    return m_pointers[index].get();
}

void ChildArrayFieldDirectStorageAccessor::insert( size_t index, std::unique_ptr<ObjectHandle> pointer )
{
    CAFFA_ASSERT( pointer );

    auto it = m_pointers.begin() + index;
    m_pointers.insert( it, std::move( pointer ) );
}
void ChildArrayFieldDirectStorageAccessor::push_back( std::unique_ptr<ObjectHandle> pointer )
{
    m_pointers.push_back( std::move( pointer ) );
}
size_t ChildArrayFieldDirectStorageAccessor::index( const ObjectHandle* object ) const
{
    CAFFA_ASSERT( object );
    auto it =
        std::find_if( m_pointers.begin(), m_pointers.end(), [object]( const auto& ptr ) { return ptr.get() == object; } );
    return it - m_pointers.begin();
}

std::unique_ptr<ObjectHandle> ChildArrayFieldDirectStorageAccessor::remove( size_t index )
{
    CAFFA_ASSERT( index < m_pointers.size() );

    auto it = m_pointers.begin() + index;

    if ( it != m_pointers.end() )
    {
        std::unique_ptr<ObjectHandle> detachedPtr = std::move( *it );
        m_pointers.erase( it );
        if ( detachedPtr && m_field && m_field->ownerObject() )
        {
            detachedPtr->disconnectObserverFromAllSignals( m_field->ownerObject() );
        }
        return detachedPtr;
    }

    return nullptr;
}
