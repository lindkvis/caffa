// ##################################################################################################
//
//    Caffa
//    Copyright (C) Kontur AS
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

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObjectHandle.h"

#include <algorithm>

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
    m_pointers.clear();
}

std::vector<std::shared_ptr<ObjectHandle>> ChildArrayFieldDirectStorageAccessor::objects()
{
    return m_pointers;
}

std::vector<std::shared_ptr<const ObjectHandle>> ChildArrayFieldDirectStorageAccessor::objects() const
{
    std::vector<std::shared_ptr<const ObjectHandle>> constPointers;
    for ( auto ptr : m_pointers )
    {
        constPointers.push_back( std::dynamic_pointer_cast<const ObjectHandle>( ptr ) );
    }
    return constPointers;
}

std::shared_ptr<ObjectHandle> ChildArrayFieldDirectStorageAccessor::at( size_t index ) const
{
    CAFFA_ASSERT( index < m_pointers.size() );
    return m_pointers[index];
}

void ChildArrayFieldDirectStorageAccessor::insert( size_t index, std::shared_ptr<ObjectHandle> pointer )
{
    CAFFA_ASSERT( pointer );

    auto it = m_pointers.begin() + index;
    m_pointers.insert( it, pointer );
}

void ChildArrayFieldDirectStorageAccessor::push_back( std::shared_ptr<ObjectHandle> pointer )
{
    m_pointers.push_back( pointer );
}

size_t ChildArrayFieldDirectStorageAccessor::index( std::shared_ptr<const ObjectHandle> object ) const
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
        m_pointers.erase( it );
    }
    else
    {
        throw std::runtime_error( "Index out of range " + std::to_string( index ) );
    }
}
