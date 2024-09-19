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

#include "cafFieldValidator.h"
#include "cafJsonDefinitions.h"

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
class RangeValidator final : public FieldValidator<DataType>
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
        auto jsonFieldValue = json::parse( string );
        if ( const auto* jsonFieldObject = jsonFieldValue.if_object(); jsonFieldObject )
        {
            if ( const auto it = jsonFieldObject->find( "minimum" ); it != jsonFieldObject->end() )
            {
                m_minimum = json::from_json<DataType>( it->value() );
            }
            if ( const auto it = jsonFieldObject->find( "maximum" ); it != jsonFieldObject->end() )
            {
                m_maximum = json::from_json<DataType>( it->value() );
            }
        }
    }

    [[nodiscard]] std::string writeToString() const override
    {
        const json::object jsonFieldObject = { { "minimum", json::to_json( m_minimum ) },
                                               { "maximum", json::to_json( m_maximum ) } };
        return json::dump( jsonFieldObject );
    }

    [[nodiscard]] std::pair<bool, std::string> validate( const DataType& value ) const override
    {
        if ( value < m_minimum || value > m_maximum )
        {
            std::stringstream ss;
            ss << "The value " << value << " is outside the limits [" << m_minimum << ", " << m_maximum << "]";
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    [[nodiscard]] static std::unique_ptr<RangeValidator>
        create( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
    {
        return std::make_unique<RangeValidator>( minimum, maximum, failureSeverity );
    }

private:
    DataType m_minimum;
    DataType m_maximum;
};

} // namespace caffa