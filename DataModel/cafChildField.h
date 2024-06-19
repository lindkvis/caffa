// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013- Ceetron Solutions AS
//    Copyright (C) 2022- Kontur AS
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
#pragma once

#include "cafAssert.h"
#include "cafChildFieldAccessor.h"
#include "cafChildFieldHandle.h"
#include "cafFieldHandle.h"
#include "cafObjectHandle.h"
#include "cafObjectHandlePortableDataType.h"
#include "cafVisitor.h"

#include <concepts>
#include <memory>
#include <type_traits>

namespace caffa
{

/**
 * @brief Field class to handle a pointer to a caffa Object.
 *
 * @tparam DataTypePtr A pointer to a class derived from caffa::Object
 */
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
class ChildField : public ChildFieldHandle
{
public:
    using DataType      = typename std::remove_pointer<DataTypePtr>::type;
    using Ptr           = std::shared_ptr<DataType>;
    using ConstPtr      = std::shared_ptr<const DataType>;
    using FieldDataType = DataTypePtr;

public:
    ChildField()
        : m_fieldDataAccessor( std::make_unique<ChildFieldDirectStorageAccessor>( this ) )
    {
        static_assert( DerivesFromObjectHandle<DataType> &&
                       "Child fields can only contain ObjectHandle-derived objects" );
    }

    virtual ~ChildField();

    bool empty() const override { return !object(); }

    // Assignment

    ChildField& operator=( Ptr object );
    bool        operator==( ObjectHandle::ConstPtr object ) const;
    bool        operator==( const ObjectHandle* object ) const;

    // Basic access

    std::shared_ptr<DataType> object()
    {
        if ( !m_fieldDataAccessor )
        {
            std::string errorMessage = "Failed to get object for '" + this->keyword() + "': Field is not accessible";
            CAFFA_ERROR( errorMessage );
            throw std::runtime_error( errorMessage );
        }

        return std::dynamic_pointer_cast<DataType>( m_fieldDataAccessor->object() );
    }
    std::shared_ptr<const DataType> object() const
    {
        if ( !m_fieldDataAccessor )
        {
            std::string errorMessage = "Failed to get object for '" + this->keyword() + "': Field is not accessible";
            CAFFA_ERROR( errorMessage );
            throw std::runtime_error( errorMessage );
        }

        return std::dynamic_pointer_cast<const DataType>( m_fieldDataAccessor->object() );
    }
    void setObject( Ptr object );

    // Access operators
    operator std::shared_ptr<DataType>() { return this->object(); }
    operator std::shared_ptr<const DataType>() const { return this->object(); }

    operator bool() const { return !!this->object(); }

    std::shared_ptr<DataType>       operator->() { return this->object(); }
    std::shared_ptr<const DataType> operator->() const { return this->object(); }

    std::shared_ptr<DataType>       operator()() { return this->object(); }
    std::shared_ptr<const DataType> operator()() const { return this->object(); }

    // Child objects
    std::vector<ObjectHandle::Ptr>      childObjects() override;
    std::vector<ObjectHandle::ConstPtr> childObjects() const override;
    void                                clear() override;
    void                                removeChildObject( ObjectHandle::ConstPtr object );
    void                                setChildObject( ObjectHandle::Ptr object ) override;

    std::string dataType() const override { return PortableDataType<DataType>::name(); }

    bool isReadable() const override { return m_fieldDataAccessor != nullptr && m_fieldDataAccessor->hasGetter(); }
    bool isWritable() const override { return m_fieldDataAccessor != nullptr && m_fieldDataAccessor->hasSetter(); }

    void setAccessor( std::unique_ptr<ChildFieldAccessor> accessor ) override
    {
        m_fieldDataAccessor = std::move( accessor );
    }

    std::string childClassKeyword() const override { return std::string( DataType::classKeywordStatic() ); }

private:
    ChildField( const ChildField& )            = delete;
    ChildField& operator=( const ChildField& ) = delete;

    mutable std::unique_ptr<ChildFieldAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildField.inl"
