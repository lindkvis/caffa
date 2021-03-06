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
#ifndef CAF_IS_DEFINING_PDM_FIELD
#pragma once
#endif

#include "cafValueField.h"

#include "cafAssert.h"
#include "cafDataFieldAccessor.h"
#include "cafFieldUiCapabilityInterface.h"
#include "cafValueFieldSpecializations.h"

#include <any>
#include <typeinfo>
#include <vector>

namespace caf
{
class ObjectHandle;

//==================================================================================================
/// Field class encapsulating data with input and output of this data to/from a QXmlStream
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================

template <typename DataType>
class DataValueField : public ValueField
{
public:
    // Type traits magic to check if a template argument is a vector
    template <typename T>
    struct is_vector : public std::false_type
    {
    };
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type
    {
    };

    using FieldDataType         = DataType;
    using DataAccessor          = DataFieldAccessorInterface<DataType>;
    using DirectStorageAccessor = DataFieldDirectStorageAccessor<DataType>;

    DataValueField()
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>() )
    {
    }
    DataValueField( const DataValueField& other ) { m_fieldDataAccessor = other.m_fieldDataAccessor; }
    explicit DataValueField( const DataType& fieldValue )
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>( fieldValue ) )
    {
    }
    ~DataValueField() override {}

    // Assignment

    DataValueField& operator=( const DataValueField& other )
    {
        CAF_ASSERT( isInitializedByInitFieldMacro() );
        m_fieldDataAccessor = other.m_fieldDataAccessor->clone();
        return *this;
    }
    DataValueField& operator=( const DataType& fieldValue )
    {
        CAF_ASSERT( isInitializedByInitFieldMacro() );
        m_fieldDataAccessor->setValue( fieldValue );
        return *this;
    }

    // Basic access

    DataType& value() const { return m_fieldDataAccessor->value(); }
    void      setValue( const DataType& fieldValue )
    {
        CAF_ASSERT( isInitializedByInitFieldMacro() );
        m_fieldDataAccessor->setValue( fieldValue );
    }
    void setValueWithFieldChanged( const DataType& fieldValue );

    // Implementation of ValueField interface

    Variant toVariant() const override
    {
        CAF_ASSERT( isInitializedByInitFieldMacro() );
        return ValueFieldSpecialization<DataType>::convert( m_fieldDataAccessor->value() );
    }
    void setFromVariant( const Variant& variant ) override
    {
        CAF_ASSERT( isInitializedByInitFieldMacro() );
        ValueFieldSpecialization<DataType>::setFromVariant( variant, m_fieldDataAccessor->value() );
    }
    bool isReadOnly() const override { return false; }

    // Access operators

    /*Conversion */ operator DataType() const { return m_fieldDataAccessor->value(); }
    const DataType& operator()() const { return m_fieldDataAccessor->value(); }

    DataType&       v() { return m_fieldDataAccessor->value(); }
    const DataType& v() const { return m_fieldDataAccessor->value(); }

    bool operator==( const DataType& fieldValue ) const { return m_fieldDataAccessor->value() == fieldValue; }
    bool operator!=( const DataType& fieldValue ) const { return m_fieldDataAccessor->value() != fieldValue; }

    // Replace accessor
    void setFieldDataAccessor( std::unique_ptr<DataAccessor> accessor )
    {
        // Copy existing values
        accessor->setValue( m_fieldDataAccessor->value() );
        if ( accessor->defaultValue().has_value() ) accessor->setDefaultValue( m_fieldDataAccessor->defaultValue() );
        // Assign new accessor
        m_fieldDataAccessor = std::move( accessor );
    }

public:
    std::optional<DataType> defaultValue() const { return m_fieldDataAccessor->defaultValue(); }
    void                    setDefaultValue( const DataType& val ) { m_fieldDataAccessor->setDefaultValue( val ); }

protected:
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::DataValueField<DataType>::setValueWithFieldChanged( const DataType& fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    FieldUiCapabilityInterface* uiFieldHandleInterface = capability<FieldUiCapabilityInterface>();
    if ( uiFieldHandleInterface )
    {
        Variant oldValue = uiFieldHandleInterface->toUiBasedVariant();

        m_fieldDataAccessor->setValue( fieldValue );

        Variant newUiBasedVariant = uiFieldHandleInterface->toUiBasedVariant();

        uiFieldHandleInterface->notifyFieldChanged( oldValue, newUiBasedVariant );
    }
    else
    {
        m_fieldDataAccessor->setValue( fieldValue );
    }
}

} // End of namespace caf
