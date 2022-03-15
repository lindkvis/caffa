//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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
#pragma once

#include <memory>
#include <optional>

namespace caffa
{
/**
 * @brief Basic non-typed interface which exists only to allow non-typed pointers.
 *
 */
class DataFieldAccessorInterface
{
public:
    virtual ~DataFieldAccessorInterface() = default;
};

/**
 * @brief Abstract but typed data field accessor. Inherit to create different storage mechanisms.
 *
 * @tparam DataType
 */
template <class DataType>
class DataFieldAccessor : public DataFieldAccessorInterface
{
public:
    /**
     * @brief Clone the accessor using polymorphism
     *
     * @return A unique pointer to object of implementation class.
     */
    virtual std::unique_ptr<DataFieldAccessor<DataType>> clone() const = 0;

    /**
     * @brief Get the field value
     *
     * @return Field value
     */
    virtual DataType value() = 0;

    /**
     * @brief Set the value with the accessor. Will throw a std::runtime_exception if the accessor
     * has limits and the value is outside those limits.
     *
     * @param value The value to set
     */
    virtual void setValue( const DataType& value ) = 0;

    /**
     * @brief An optional default value
     *
     * @return The default value, or std::nullopt if not set.
     */
    virtual std::optional<DataType> defaultValue() const { return std::nullopt; }

    /**
     * @brief Apply an optional default value to  the field.
     *
     */
    virtual void setDefaultValue( const DataType& ) {}

    /**
     * @brief Get the limits for the data values (if they exist) or std::nullopt if not.
     *
     * @return A pair of minimum and maximum if there are limits set.
     */
    virtual std::optional<std::pair<DataType, DataType>> limits() const { return std::nullopt; }

    /**
     * @brief Set limits to the data value. If the limits are set, any attempt to set a value
     * outside those limits should result in an exception.
     *
     * @param minimum Minimum acceptable value
     * @param maximum Maximum acceptable value
     */
    virtual void setLimits( const DataType&, const DataType& ) {}
};

/**
 * @brief Direct storage accessor, which stores data values in local memory.
 *
 * @tparam DataType
 */
template <class DataType>
class DataFieldDirectStorageAccessor : public DataFieldAccessor<DataType>
{
public:
    /**
     * @brief Construct a new Data Field Direct Storage Accessor object
     *
     */
    DataFieldDirectStorageAccessor() = default;

    /**
     * @brief Construct a new Data Field Direct Storage Accessor object with a default value
     *
     * @param defaultValue Default value
     */
    DataFieldDirectStorageAccessor( const DataType& defaultValue )
        : m_value( defaultValue )
        , m_defaultValue( defaultValue )

    {
    }

    std::unique_ptr<DataFieldAccessor<DataType>> clone() const override
    {
        std::unique_ptr<DataFieldAccessor<DataType>> copy;
        if ( m_defaultValue.has_value() )
            copy = std::make_unique<DataFieldDirectStorageAccessor<DataType>>( *m_defaultValue );
        else
            copy = std::make_unique<DataFieldDirectStorageAccessor<DataType>>();

        copy->setValue( m_value );
        return copy;
    }

    DataType value() override { return m_value; };

    void setValue( const DataType& value ) override { m_value = value; }

    std::optional<DataType> defaultValue() const override { return m_defaultValue; }

    /**
     * @brief Apply an optional default value to  the field.
     * @param value The default value.
     *
     */
    void setDefaultValue( const DataType& value ) override { m_defaultValue = value; }

    std::optional<std::pair<DataType, DataType>> limits() const override { return m_limits; }

    /**
     * @brief Set limits to the data value. If the limits are set, any attempt to set a value
     * outside those limits should result in an exception.
     *
     * @param minimum Minimum acceptable value
     * @param maximum Maximum acceptable value
     */
    void setLimits( const DataType& minimum, const DataType& maximum ) override
    {
        m_limits = std::make_pair( minimum, maximum );
    }

private:
    DataType                                     m_value;
    std::optional<DataType>                      m_defaultValue;
    std::optional<std::pair<DataType, DataType>> m_limits;
};

} // namespace caffa
