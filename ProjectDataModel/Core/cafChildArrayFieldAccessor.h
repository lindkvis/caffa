//##################################################################################################
//
//   Caffa
//   Copyright (C) 3D-Radar AS
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
#pragma once

#include "cafAssert.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

namespace caffa
{
class FieldHandle;
class ObjectHandle;

template <class DataType>
class ChildArrayFieldAccessor
{
public:
    using DataTypeUniquePtr = std::unique_ptr<DataType>;

    ChildArrayFieldAccessor( FieldHandle* field )
        : m_field( field )
    {
    }
    virtual ~ChildArrayFieldAccessor()                           = default;
    virtual size_t                                 size() const  = 0;
    virtual std::vector<std::unique_ptr<DataType>> removeAll()   = 0;
    virtual std::vector<DataType*>                 value() const = 0;

    virtual DataType*                 at( size_t index )                                = 0;
    virtual void                      insert( size_t index, DataTypeUniquePtr pointer ) = 0;
    virtual void                      push_back( DataTypeUniquePtr pointer )            = 0;
    virtual size_t                    index( const ObjectHandle* pointer ) const        = 0;
    virtual std::unique_ptr<DataType> remove( size_t index )                            = 0;

    virtual std::vector<ObjectHandle*> childObjects() const = 0;

protected:
    FieldHandle* m_field;
};

template <class DataType>
class ChildArrayFieldDirectStorageAccessor : public ChildArrayFieldAccessor<DataType>
{
public:
    using DataTypeUniquePtr = typename ChildArrayFieldAccessor<DataType>::DataTypeUniquePtr;

    ChildArrayFieldDirectStorageAccessor( FieldHandle* field )
        : ChildArrayFieldAccessor<DataType>( field )
    {
    }
    ~ChildArrayFieldDirectStorageAccessor() override = default;

    size_t                                 size() const override { return m_pointers.size(); }
    std::vector<std::unique_ptr<DataType>> removeAll() override
    {
        for ( auto& ptr : m_pointers )
        {
            ptr->detachFromParentField();
        }

        std::vector<std::unique_ptr<DataType>> returnValues;
        returnValues.swap( m_pointers );

        return returnValues;
    }

    std::vector<DataType*> value() const override
    {
        std::vector<DataType*> rawPointers;
        for ( auto& ptr : m_pointers )
        {
            rawPointers.push_back( ptr.get() );
        }
        return rawPointers;
    }

    DataType* at( size_t index ) override
    {
        CAFFA_ASSERT( index < m_pointers.size() );
        return m_pointers[index].get();
    }

    void insert( size_t index, DataTypeUniquePtr pointer ) override
    {
        CAFFA_ASSERT( pointer );

        pointer->setAsParentField( this->m_field );
        auto it = m_pointers.begin() + index;
        m_pointers.insert( it, std::move( pointer ) );
    }
    void push_back( DataTypeUniquePtr pointer ) override
    {
        pointer->setAsParentField( this->m_field );
        m_pointers.push_back( std::move( pointer ) );
    }
    size_t index( const ObjectHandle* object ) const override
    {
        CAFFA_ASSERT( object );
        auto it = std::find_if( m_pointers.begin(),
                                m_pointers.end(),
                                [object]( const auto& ptr ) { return ptr.get() == object; } );
        return it - m_pointers.begin();
    }

    std::unique_ptr<DataType> remove( size_t index ) override
    {
        CAFFA_ASSERT( index < m_pointers.size() );

        auto it = m_pointers.begin() + index;

        if ( it != m_pointers.end() )
        {
            std::unique_ptr<DataType> detachedPtr = std::move( *it );
            m_pointers.erase( it );
            detachedPtr->detachFromParentField();
            return detachedPtr;
        }

        return nullptr;
    }

    std::vector<ObjectHandle*> childObjects() const override
    {
        std::vector<ObjectHandle*> objects;
        for ( auto& object : m_pointers )
        {
            objects.push_back( object.get() );
        }
        return objects;
    }

private:
    std::vector<std::unique_ptr<DataType>> m_pointers;
};

} // namespace caffa
