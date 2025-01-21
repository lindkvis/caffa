// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2025- Kontur AS
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
 * @brief Simple vector size validator.
 *
 * @tparam DataType vector item data type
 */
template <typename DataType>
class VectorSizeValidator final : public FieldValidator<std::vector<DataType>>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    VectorSizeValidator( const size_t    minimumSize,
                         const size_t    maximumSize,
                         FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<std::vector<DataType>>( failureSeverity )
        , m_minimumSize( minimumSize )
        , m_maximumSize( maximumSize )
    {
    }

    void readFromString( const std::string& string ) override
    {
        auto jsonFieldValue = json::parse( string );
        if ( const auto* jsonFieldObject = jsonFieldValue.if_object(); jsonFieldObject )
        {
            if ( const auto it = jsonFieldObject->find( "minItems" ); it != jsonFieldObject->end() )
            {
                m_minimumSize = json::from_json<size_t>( it->value() );
            }
            if ( const auto it = jsonFieldObject->find( "maxItems" ); it != jsonFieldObject->end() )
            {
                m_maximumSize = json::from_json<size_t>( it->value() );
            }
        }
    }

    [[nodiscard]] std::string writeToString() const override
    {
        const json::object jsonFieldObject = { { "minItems", json::to_json( m_minimumSize ) },
                                               { "maxItems", json::to_json( m_maximumSize ) } };
        return json::dump( jsonFieldObject );
    }

    [[nodiscard]] std::pair<bool, std::string> validate( const std::vector<DataType>& vector ) const override
    {
        if ( vector.size() < m_minimumSize || vector.size() > m_maximumSize )
        {
            std::stringstream ss;
            ss << "The size " << vector.size() << " is outside the limits [" << m_minimumSize << ", " << m_maximumSize
               << "]";
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    [[nodiscard]] static std::unique_ptr<VectorSizeValidator>
        create( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
    {
        return std::make_unique<VectorSizeValidator>( minimum, maximum, failureSeverity );
    }

private:
    size_t m_minimumSize;
    size_t m_maximumSize;
};

} // namespace caffa