// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
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
     * The accessor has a getter. Thus can be read.
     * @return true if it has a getter
     */
    virtual bool hasSetter() const = 0;

    /**
     * The accessor has a setter. Thus can be written to.
     * @return true if it has a setter
     */
    virtual bool hasGetter() const = 0;
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
     * @param value Default value
     */
    DataFieldDirectStorageAccessor( const DataType& value )
        : m_value( value )

    {
    }

    std::unique_ptr<DataFieldAccessor<DataType>> clone() const override
    {
        return std::make_unique<DataFieldDirectStorageAccessor<DataType>>( m_value );
    }

    DataType value() override { return m_value; };

    void setValue( const DataType& value ) override { m_value = value; }

    bool hasGetter() const { return true; }
    bool hasSetter() const { return true; }

private:
    DataType m_value;
};

} // namespace caffa
