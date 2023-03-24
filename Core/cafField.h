// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2021 3D-Radar AS
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

#include "cafDynamicUniqueCast.h"
#include "cafFieldValidator.h"
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

    explicit Field( const Field& other ) = delete;

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

    Field& operator=( const Field& other ) = delete;

    Field& operator=( const DataType& fieldValue )
    {
        this->setValue( fieldValue );
        return *this;
    }

    // Basic access

    DataType value() const override
    {
        CAFFA_ASSERT( this->isInitialized() );

        try
        {
            return m_fieldDataAccessor->value();
        }
        catch ( const std::exception& e )
        {
            std::string errorMessage = "Failed to get value for '" + this->keyword() + "': " + e.what();
            CAFFA_ERROR( errorMessage );
            throw std::runtime_error( errorMessage );
        }
    }
    void setValue( const DataType& fieldValue ) override
    {
        CAFFA_ASSERT( this->isInitialized() );

        try
        {
            for ( const auto& validator : m_valueValidators )
            {
                if ( auto [status, message] = validator->validate( fieldValue ); !status )
                {
                    CAFFA_ASSERT( !message.empty() );
                    if ( validator->failureSeverity() == FieldValidatorInterface::FailureSeverity::VALIDATOR_ERROR ||
                         validator->failureSeverity() == FieldValidatorInterface::FailureSeverity::VALIDATOR_CRITICAL )
                    {
                        throw std::runtime_error( message );
                    }
                    else
                    {
                        CAFFA_WARNING( message );
                    }
                }
            }
            m_fieldDataAccessor->setValue( fieldValue );
        }
        catch ( const std::exception& e )
        {
            std::string errorMessage = "Failed to set value for '" + this->keyword() + "': " + e.what();
            CAFFA_ERROR( errorMessage );
            throw std::runtime_error( errorMessage );
        }
    }

    // Access operators

    /*Conversion */ operator DataType() const
    {
        CAFFA_ASSERT( m_fieldDataAccessor );
        return this->value();
    }
    DataType operator()() const
    {
        CAFFA_ASSERT( m_fieldDataAccessor );
        return this->value();
    }

    bool operator==( const Field<DataType>& rhs ) const { return this->value() == rhs.value(); }
    auto operator<=>( const Field<DataType>& rhs ) const { return this->value() <=> rhs.value(); }

    bool operator==( const DataType& fieldValue ) const { return this->value() == fieldValue; }
    auto operator<=>( const DataType& fieldValue ) const { return this->value() <=> fieldValue; }

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

    std::vector<const FieldValidator<DataType>*> valueValidators() const
    {
        std::vector<const FieldValidator<DataType>*> allValidators;
        for ( const auto& validator : m_valueValidators )
        {
            allValidators.push_back( validator.get() );
        }
        return allValidators;
    }

    std::vector<FieldValidator<DataType>*> valueValidators()
    {
        std::vector<FieldValidator<DataType>*> allValidators;
        for ( auto& validator : m_valueValidators )
        {
            allValidators.push_back( validator.get() );
        }
        return allValidators;
    }

    void addValidator( std::unique_ptr<FieldValidator<DataType>> valueValidator )
    {
        m_valueValidators.push_back( std::move( valueValidator ) );
    }

    void clearValidators() { m_valueValidators.clear(); }

public:
    std::optional<DataType> defaultValue() const { return m_defaultValue; }
    void                    setDefaultValue( const DataType& val ) { m_defaultValue = val; }

    void resetToDefault() override
    {
        if ( m_defaultValue )
        {
            this->setValue( *m_defaultValue );
        }
    }

protected:
    std::unique_ptr<DataAccessor>                          m_fieldDataAccessor;
    std::vector<std::unique_ptr<FieldValidator<DataType>>> m_valueValidators;
    std::optional<DataType>                                m_defaultValue;
};

} // End of namespace caffa
