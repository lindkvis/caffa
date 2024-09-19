// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2022- Kontur AS
//
//    This library may be used under the terms of the GNU Lesser General Public License as follows:
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

#include "cafFieldValidator.h"
#include "cafJsonDefinitions.h"

#include <set>

namespace caffa
{
/**
 * @brief Simple divisible by int-validator.
 *
 */
template <typename DataType>
class DivisibleByValidator final : public FieldValidator<DataType>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    explicit DivisibleByValidator( DataType divisor, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<DataType>( failureSeverity )
        , m_divisor( divisor )
    {
    }

    void readFromString( const std::string& string ) override
    {
        auto jsonValue = json::parse( string );
        if ( const auto jsonObject = jsonValue.if_object(); jsonObject )
        {
            if ( const auto it = jsonObject->find( "valid-divisor" ); it != jsonObject->end() )
            {
                const auto jsonDivisor = it->value().as_object();

                if ( const auto jt = jsonDivisor.find( "divisor" ); jt != jsonDivisor.end() )
                {
                    m_divisor = json::from_json<DataType>( jt->value() );
                }
            }
        }
    }

    [[nodiscard]] std::string writeToString() const override
    {
        auto jsonObject             = json::object();
        auto jsonDivisor            = json::object();
        jsonDivisor["divisor"]      = json::to_json( m_divisor );
        jsonObject["valid-divisor"] = jsonDivisor;
        return json::dump( jsonObject );
    }

    [[nodiscard]] std::pair<bool, std::string> validate( const DataType& value ) const override
    {
        if ( ( value % m_divisor ) != DataType( 0 ) )
        {
            std::stringstream ss;
            ss << "The value " << value << " is not divisible by " << m_divisor;
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    [[nodiscard]] static std::unique_ptr<DivisibleByValidator> create( DataType divisor )
    {
        return std::make_unique<DivisibleByValidator>( divisor );
    }

private:
    DataType m_divisor;
};

/**
 * @brief Simple legal values validator
 *
 */
template <typename DataType>
class LegalValuesValidator final : public FieldValidator<DataType>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    explicit LegalValuesValidator( const std::set<DataType>& legalValues,
                                   FailureSeverity           failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<DataType>( failureSeverity )
        , m_legalValues( legalValues )
    {
    }

    void readFromString( const std::string& string ) override
    {
        const auto jsonValue = json::parse( string );
        if ( const auto* jsonObject = jsonValue.if_object(); jsonObject )
        {
            if ( const auto it = jsonObject->find( "valid-legal" ); it != jsonObject->end() )
            {
                m_legalValues = json::from_json<std::set<DataType>>( it->value() );
            }
        }
    }

    [[nodiscard]] std::string writeToString() const override
    {
        json::object jsonObject;

        jsonObject["valid-legal"] = { { "values", json::to_json( m_legalValues ) } };
        return json::dump( jsonObject );
    }

    std::pair<bool, std::string> validate( const DataType& value ) const override
    {
        if ( !m_legalValues.contains( value ) )
        {
            std::stringstream ss;
            int               index = 0;
            ss << "The value " << value << " is not one of the legal values [";
            for ( auto legalValue : m_legalValues )
            {
                if ( index > 0 ) ss << ", ";
                ss << legalValue;
                index++;
            }
            ss << "]";
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    static std::unique_ptr<LegalValuesValidator> create( const std::set<DataType>& legalValues )
    {
        return std::make_unique<LegalValuesValidator>( legalValues );
    }

private:
    std::set<DataType> m_legalValues;
};

/**
 * @brief Simple legal values validator for vector entries
 *
 */
template <typename DataType>
class LegalVectorValuesValidator final : public FieldValidator<std::vector<DataType>>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    explicit LegalVectorValuesValidator( const std::set<DataType>& legalValues,
                                         FailureSeverity           failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<std::vector<DataType>>( failureSeverity )
        , m_legalValues( legalValues )
    {
    }

    void readFromString( const std::string& string ) override
    {
        const auto jsonValue = json::parse( string );
        if ( const auto* jsonObject = jsonValue.if_object(); jsonObject )
        {
            if ( const auto it = jsonObject->find( "valid-legal" ); it != jsonObject->end() )
            {
                m_legalValues = json::from_json<std::set<DataType>>( it->value() );
            }
        }
    }

    [[nodiscard]] std::string writeToString() const override
    {
        json::object jsonObject;

        jsonObject["valid-legal"] = { { "values", json::to_json( m_legalValues ) } };
        return json::dump( jsonObject );
    }

    std::pair<bool, std::string> validate( const std::vector<DataType>& values ) const override
    {
        for ( const auto& value : values )
        {
            if ( !m_legalValues.contains( value ) )
            {
                std::stringstream ss;
                int               index = 0;
                ss << "The value " << value << " is not one of the legal values [";
                for ( auto legalValue : m_legalValues )
                {
                    if ( index > 0 ) ss << ", ";
                    ss << legalValue;
                    index++;
                }
                ss << "]";
                return std::make_pair( false, ss.str() );
            }
        }
        return std::make_pair( true, "" );
    }

    static std::unique_ptr<LegalVectorValuesValidator> create( const std::set<DataType>& legalValues )
    {
        return std::make_unique<LegalVectorValuesValidator>( legalValues );
    }

private:
    std::set<DataType> m_legalValues;
};

/**
 * @brief Simple illegal values validator
 *
 */
template <typename DataType>
class IllegalValuesValidator final : public FieldValidator<DataType>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    explicit IllegalValuesValidator( const std::set<DataType>& illegalValues,
                                     FailureSeverity           failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<DataType>( failureSeverity )
        , m_illegalValues( illegalValues )
    {
    }

    void readFromString( const std::string& string ) override
    {
        const auto jsonValue = json::parse( string );
        if ( const auto* jsonObject = jsonValue.if_object(); jsonObject )
        {
            if ( const auto it = jsonObject->find( "valid-illegal" ); it != jsonObject->end() )
            {
                m_illegalValues = json::from_json<std::set<DataType>>( it->value() );
            }
        }
    }

    [[nodiscard]] std::string writeToString() const override
    {
        json::object jsonObject;

        jsonObject["valid-illegal"] = { { "values", json::to_json( m_illegalValues ) } };
        return json::dump( jsonObject );
    }

    [[nodiscard]] std::pair<bool, std::string> validate( const DataType& value ) const override
    {
        if ( m_illegalValues.count( value ) )
        {
            std::stringstream ss;
            int               index = 0;
            ss << "The value " << value << " is one of the illegal values [";
            for ( auto illegalValue : m_illegalValues )
            {
                if ( index > 0 ) ss << ", ";
                ss << illegalValue;
                index++;
            }
            ss << "]";
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    [[nodiscard]] static std::unique_ptr<IllegalValuesValidator> create( const std::set<DataType>& illegalValues )
    {
        return std::make_unique<IllegalValuesValidator>( illegalValues );
    }

private:
    std::set<DataType> m_illegalValues;
};
} // namespace caffa