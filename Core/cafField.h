//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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
//##################################################################################################
#pragma once

#include "cafValueField.h"

#include "cafAssert.h"
#include "cafDataFieldAccessor.h"

#include <any>
#include <vector>

namespace caffa
{
class ObjectHandle;

//==================================================================================================
/// Field class encapsulating data with input and output of this data to/from JSON
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================

template <typename DataType>
class Field : public TypedValueField<DataType>
{
public:
    typedef DataFieldAccessor<DataType>              DataAccessor;
    typedef DataFieldDirectStorageAccessor<DataType> DirectStorageAccessor;

    Field()
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>() )
    {
    }
    Field( const Field& other ) { m_fieldDataAccessor = std::move( other.m_fieldDataAccessor->clone() ); }
    explicit Field( const DataType& fieldValue )
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>( fieldValue ) )
    {
    }

    Field( std::unique_ptr<DataAccessor> accessor )
        : m_fieldDataAccessor( std::move( accessor ) )
    {
    }
    ~Field() noexcept override {}

    // Assignment

    Field& operator=( const Field& other )
    {
        m_fieldDataAccessor = std::move( other.m_fieldDataAccessor->clone() );
        return *this;
    }
    Field& operator=( const DataType& fieldValue )
    {
        CAFFA_ASSERT( this->isInitializedByInitFieldMacro() );
        m_fieldDataAccessor->setValue( fieldValue );
        return *this;
    }

    // Basic access

    DataType value() const override { return m_fieldDataAccessor->value(); }
    void     setValue( const DataType& fieldValue ) override
    {
        CAFFA_ASSERT( this->isInitializedByInitFieldMacro() );
        m_fieldDataAccessor->setValue( fieldValue );
    }

    // Implementation of ValueField interface

    bool isReadOnly() const override { return false; }

    // Access operators

    /*Conversion */ operator DataType() const
    {
        CAFFA_ASSERT( m_fieldDataAccessor );
        return m_fieldDataAccessor->value();
    }
    DataType operator()() const
    {
        CAFFA_ASSERT( m_fieldDataAccessor );
        return m_fieldDataAccessor->value();
    }

    bool operator==( const DataType& fieldValue ) const { return m_fieldDataAccessor->value() == fieldValue; }
    bool operator!=( const DataType& fieldValue ) const { return m_fieldDataAccessor->value() != fieldValue; }

    // Replace accessor
    void setAccessor( std::unique_ptr<DataAccessor> accessor ) { m_fieldDataAccessor = std::move( accessor ); }

public:
    std::optional<DataType> defaultValue() const { return m_fieldDataAccessor->defaultValue(); }
    void                    setDefaultValue( const DataType& val ) { m_fieldDataAccessor->setDefaultValue( val ); }

protected:
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa
