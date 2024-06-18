// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2021- 3D-Radar AS
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

#include "cafAssert.h"

#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace caffa
{

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
     * VALIDATOR_WARNING -> user warning
     * VALIDATOR_ERROR -> user error
     * VALIDATOR_CRITICAL -> critical application failure
     */
    enum class FailureSeverity
    {
        VALIDATOR_WARNING,
        VALIDATOR_ERROR,
        VALIDATOR_CRITICAL
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
     * @brief Read the validator from string.
     *
     * @param string the string to read from
     */
    virtual void readFromString( const std::string& string ) = 0;

    /**
     * @brief Write the validator tostring.
     *
     * @return a string containing the validator info
     */
    virtual std::string writeToString() const = 0;

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
 * Implementations need the the validate method as well as readFromString and writeToString.
 *
 * @tparam DataType
 */
template <typename DataType>
class FieldValidator : public FieldValidatorInterface
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    FieldValidator( FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
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

} // namespace caffa