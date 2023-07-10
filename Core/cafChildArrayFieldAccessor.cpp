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

void ChildArrayFieldDirectStorageAccessor::clear()
{
    for ( auto& ptr : m_pointers )
    {
        ptr->disconnectObserverFromAllSignals( m_field->ownerObject() );
    }
    m_pointers.clear();
}

std::vector<ObjectHandle::Ptr> ChildArrayFieldDirectStorageAccessor::objects()
{
    return m_pointers;
}

std::vector<ObjectHandle::ConstPtr> ChildArrayFieldDirectStorageAccessor::objects() const
{
    std::vector<ObjectHandle::ConstPtr> constPointers;
    for ( auto ptr : m_pointers )
    {
        constPointers.push_back( std::dynamic_pointer_cast<const ObjectHandle>( ptr ) );
    }
    return constPointers;
}

ObjectHandle::Ptr ChildArrayFieldDirectStorageAccessor::at( size_t index ) const
{
    CAFFA_ASSERT( index < m_pointers.size() );
    return m_pointers[index];
}

void ChildArrayFieldDirectStorageAccessor::insert( size_t index, ObjectHandle::Ptr pointer )
{
    CAFFA_ASSERT( pointer );

    auto it = m_pointers.begin() + index;
    m_pointers.insert( it, pointer );
}

void ChildArrayFieldDirectStorageAccessor::push_back( ObjectHandle::Ptr pointer )
{
    m_pointers.push_back( pointer );
}

size_t ChildArrayFieldDirectStorageAccessor::index( ObjectHandle::ConstPtr object ) const
{
    auto it = std::find_if( m_pointers.begin(),
                            m_pointers.end(),
                            [object]( const auto& ptr ) { return ptr.get() == object.get(); } );
    if ( it == m_pointers.end() )
    {
        return std::numeric_limits<size_t>::infinity();
    }
    return it - m_pointers.begin();
}

void ChildArrayFieldDirectStorageAccessor::remove( size_t index )
{
    CAFFA_ASSERT( index < m_pointers.size() );

    auto it = m_pointers.begin() + index;

    if ( it != m_pointers.end() )
    {
        ObjectHandle::Ptr detachedPtr = *it;
        m_pointers.erase( it );
        if ( detachedPtr && m_field && m_field->ownerObject() )
        {
            detachedPtr->disconnectObserverFromAllSignals( m_field->ownerObject() );
        }
    }
    else
    {
        throw std::runtime_error( "Index out of range " + std::to_string( index ) );
    }
}
