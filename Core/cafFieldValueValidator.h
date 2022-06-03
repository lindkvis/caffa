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

#include <nlohmann/json.hpp>

namespace caffa
{
class Serializer;

/**
 * @brief An abstract field validator interface for validating field values
 *
 */
class FieldValueValidatorInterface
{
public:
    virtual ~FieldValueValidatorInterface() = default;
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
};

/**
 * @brief Used to validate the value of data fields
 * Implementations need the the validate method as well as readFromJson and writeToJson.
 *
 * @tparam DataType
 */
template <typename DataType>
class FieldValueValidator : public FieldValueValidatorInterface
{
public:
    /**
     * @brief Validate the value
     *
     * @param value The value to validate
     * @return true if the value is acceptable
     * @return false if not
     */
    virtual bool validate( const DataType& value ) const = 0;
};

/**
 * @brief Simple range validator.
 *
 * @tparam DataType
 */
template <typename DataType>
class RangeValueValidator : public caffa::FieldValueValidator<DataType>
{
public:
    RangeValueValidator( DataType minimum, DataType maximum )
        : m_minimum( minimum )
        , m_maximum( maximum )
    {
    }

    void readFromJson( const nlohmann::json& jsonFieldObject, const caffa::Serializer& serializer ) override
    {
        CAFFA_ASSERT( jsonFieldObject.is_object() );
        if ( jsonFieldObject.contains( "range" ) )
        {
            auto jsonRange = jsonFieldObject["range"];
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
        auto jsonRange           = nlohmann::json::object();
        jsonRange["min"]         = m_minimum;
        jsonRange["max"]         = m_maximum;
        jsonFieldObject["range"] = jsonRange;
    }
    bool validate( const DataType& value ) const override { return m_minimum <= value && value <= m_maximum; }

private:
    DataType m_minimum;
    DataType m_maximum;
};

} // namespace caffa