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
#include "cafFieldValidator.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace caffa
{
class JsonSerializer;
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

    RangeValidator( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<DataType>( failureSeverity )
        , m_minimum( minimum )
        , m_maximum( maximum )
    {
    }

    void readFromString( const std::string& string ) override
    {
        auto jsonFieldObject = nlohmann::json::parse( string );
        if ( jsonFieldObject.is_object() )
        {
            if ( jsonFieldObject.contains( "minimum" ) )
            {
                m_minimum = jsonFieldObject["minimum"];
            }
            if ( jsonFieldObject.contains( "maximum" ) )
            {
                m_maximum = jsonFieldObject["maximum"];
            }
        }
    }

    std::string writeToString() const override
    {
        auto jsonFieldObject       = nlohmann::json::object();
        jsonFieldObject["minimum"] = m_minimum;
        jsonFieldObject["maximum"] = m_maximum;
        return jsonFieldObject.dump();
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
        create( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
    {
        return std::make_unique<RangeValidator<DataType>>( minimum, maximum, failureSeverity );
    }

private:
    DataType m_minimum;
    DataType m_maximum;
};

} // namespace caffa