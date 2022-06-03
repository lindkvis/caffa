//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//   Copyright (C) 2021 3D-Radar AS
//   Copyright (C) 2022- Kontur AS
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

#include "cafDynamicUniqueCast.h"
#include "cafFieldValueValidator.h"
#include "cafTypedField.h"

#include "cafAssert.h"
#include "cafDataFieldAccessor.h"

#include "cafLogger.h"

#include <stdexcept>
#include <vector>

namespace caffa
{
class ObjectHandle;

//==================================================================================================
/// Field class encapsulating data with input and output of this data to/from JSON
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================

template <typename DataType>
class Field : public TypedField<DataType>
{
public:
    typedef DataFieldAccessor<DataType>              DataAccessor;
    typedef DataFieldDirectStorageAccessor<DataType> DirectStorageAccessor;

    Field()
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>() )
    {
    }
    Field( const Field& other )
    {
        m_fieldDataAccessor = std::move( other.m_fieldDataAccessor->clone() );
        m_defaultValue      = other.m_defaultValue;
    }
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

    DataType value() const override
    {
        CAFFA_ASSERT( this->isInitializedByInitFieldMacro() );

        try
        {
            return m_fieldDataAccessor->value();
        }
        catch ( const std::exception& e )
        {
            std::string errorMessage = "Failed to get value for '" + this->keyword() + "' due to " + e.what();
            CAFFA_ERROR( errorMessage );
            throw std::runtime_error( errorMessage );
        }
    }
    void setValue( const DataType& fieldValue ) override
    {
        CAFFA_ASSERT( this->isInitializedByInitFieldMacro() );

        try
        {
            for ( const auto& validator : m_valueValidators )
            {
                if ( !validator->validate( fieldValue ) )
                {
                    throw std::runtime_error( "An invalid value has been set!" );
                }
            }
            m_fieldDataAccessor->setValue( fieldValue );
        }
        catch ( const std::exception& e )
        {
            std::string errorMessage = "Failed to set value for '" + this->keyword() + "' due to " + e.what();
            CAFFA_ERROR( errorMessage );
            throw std::runtime_error( errorMessage );
        }
    }

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

    void setUntypedAccessor( std::unique_ptr<DataFieldAccessorInterface> accessor ) override
    {
        CAFFA_ASSERT( caffa::dynamic_unique_cast_is_valid<DataAccessor>( accessor ) );
        setAccessor( caffa::dynamic_unique_cast<DataAccessor>( std::move( accessor ) ) );
    }

    template <typename ValidatorType>
    const ValidatorType* valueValidator() const
    {
        for ( const auto& validator : m_valueValidators )
        {
            const ValidatorType* typedValidator = dynamic_cast<const ValidatorType*>( validator.get() );
            if ( typedValidator ) return typedValidator;
        }
        return nullptr;
    }

    template <typename ValidatorType>
    ValidatorType* valueValidator()
    {
        for ( auto& validator : m_valueValidators )
        {
            const ValidatorType* typedValidator = dynamic_cast<const ValidatorType*>( validator.get() );
            if ( typedValidator ) return typedValidator;
        }
        return nullptr;
    }

    std::vector<const FieldValueValidator<DataType>*> valueValidators() const
    {
        std::vector<const FieldValueValidator<DataType>*> allValidators;
        for ( const auto& validator : m_valueValidators )
        {
            allValidators.push_back( validator.get() );
        }
        return allValidators;
    }

    std::vector<FieldValueValidator<DataType>*> valueValidators()
    {
        std::vector<FieldValueValidator<DataType>*> allValidators;
        for ( auto& validator : m_valueValidators )
        {
            allValidators.push_back( validator.get() );
        }
        return allValidators;
    }

    void addValueValidator( std::unique_ptr<FieldValueValidator<DataType>> valueValidator )
    {
        m_valueValidators.push_back( std::move( valueValidator ) );
    }

public:
    std::optional<DataType> defaultValue() const { return m_defaultValue; }
    void                    setDefaultValue( const DataType& val ) { m_defaultValue = val; }

protected:
    std::unique_ptr<DataAccessor>                               m_fieldDataAccessor;
    std::vector<std::unique_ptr<FieldValueValidator<DataType>>> m_valueValidators;
    std::optional<DataType>                                     m_defaultValue;
};

} // End of namespace caffa
