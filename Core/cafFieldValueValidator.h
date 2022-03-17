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
     * @param jsonValue the JSON value to read from
     * @param serializer the serializer object
     */
    virtual void readFromJson( const nlohmann::json& jsonValue, const Serializer& serializer ) = 0;

    /**
     * @brief Write the validator to JSON.
     *
     * @param jsonValue to JSON value to write to.
     * @param serializer the serializer object
     */
    virtual void writeToJson( nlohmann::json& jsonValue, const Serializer& serializer ) const = 0;
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

} // namespace caffa