//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2021- 3D-Radar AS
//
//   This library may be used under the terms of the GNU Lesser General Public License as follows:
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

#include "cafAssert.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace caffa
{
class Serializer;

/**
 * @brief An abstract field validator interface for validating field values
 *
 */
class FieldValidatorInterface
{
public:
    /**
     * @brief The severity of failure. Essentially tells the application
     * how to treat a validator failure:
     * WARNING -> user warning
     * ERROR -> user error
     * CRITICAL -> critical application failure
     */
    enum class FailureSeverity
    {
        WARNING,
        ERROR,
        CRITICAL
    };

    /**
     * @brief Construct a new Field Validator Interface object
     *
     * @param failureSeverity the severity of a validation failure
     */
    FieldValidatorInterface( FailureSeverity failureSeverity )
        : m_failureSeverity( failureSeverity )
    {
    }

    virtual ~FieldValidatorInterface() = default;
    /**
     * @brief Read the validator from JSON.
     *
     * @param jsonFieldObject the JSON value to read from
     * @param serializer the serializer object
     */
    virtual void readFromJson( const nlohmann::json& jsonFieldObject, const Serializer& serializer ) = 0;

    /**
     * @brief Write the validator to JSON.
     *
     * @param jsonFieldObject to JSON value to write to.
     * @param serializer the serializer object
     */
    virtual void writeToJson( nlohmann::json& jsonFieldObject, const Serializer& serializer ) const = 0;

    /**
     * @brief Get the severity of a failure of the validator
     *
     * @return FailureSeverity
     */
    FailureSeverity failureSeverity() const { return m_failureSeverity; }

private:
    FailureSeverity m_failureSeverity;
};

/**
 * @brief Used to validate the value of data fields
 * Implementations need the the validate method as well as readFromJson and writeToJson.
 *
 * @tparam DataType
 */
template <typename DataType>
class FieldValidator : public FieldValidatorInterface
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    FieldValidator( FailureSeverity failureSeverity = FailureSeverity::ERROR )
        : FieldValidatorInterface( failureSeverity )
    {
    }

    /**
     * @brief Validate the value
     *
     * @param value The value to validate
     * @return true if the value is acceptable
     * @return false if not
     */
    virtual std::pair<bool, std::string> validate( const DataType& value ) const = 0;
};

/**
 * @brief Simple range validator.
 *
 * @tparam DataType
 */
template <typename DataType>
class RangeValidator : public FieldValidator<DataType>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    RangeValidator( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::ERROR )
        : FieldValidator<DataType>( failureSeverity )
        , m_minimum( minimum )
        , m_maximum( maximum )
    {
    }

    void readFromJson( const nlohmann::json& jsonFieldObject, const caffa::Serializer& serializer ) override
    {
        if ( jsonFieldObject.is_object() && jsonFieldObject.contains( "valid-range" ) )
        {
            auto jsonRange = jsonFieldObject["valid-range"];
            CAFFA_ASSERT( jsonRange.is_object() );
            if ( jsonRange.contains( "min" ) && jsonRange.contains( "max" ) )
            {
                m_minimum = jsonRange["min"];
                m_maximum = jsonRange["max"];
            }
        }
    }

    void writeToJson( nlohmann::json& jsonFieldObject, const caffa::Serializer& serializer ) const override
    {
        CAFFA_ASSERT( jsonFieldObject.is_object() );
        auto jsonRange                 = nlohmann::json::object();
        jsonRange["min"]               = m_minimum;
        jsonRange["max"]               = m_maximum;
        jsonFieldObject["valid-range"] = jsonRange;
    }

    std::pair<bool, std::string> validate( const DataType& value ) const override
    {
        bool valid = m_minimum <= value && value <= m_maximum;
        if ( !valid )
        {
            std::stringstream ss;
            ss << "The value " << value << " is outside the limits [" << m_minimum << ", " << m_maximum << "]";
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    static std::unique_ptr<RangeValidator<DataType>>
        create( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::ERROR )
    {
        return std::make_unique<RangeValidator<DataType>>( minimum, maximum, failureSeverity );
    }

private:
    DataType m_minimum;
    DataType m_maximum;
};

} // namespace caffa