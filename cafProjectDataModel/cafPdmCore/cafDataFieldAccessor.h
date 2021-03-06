//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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

namespace caf
{
template <class DataType>
class DataFieldAccessorInterface
{
public:
    virtual ~DataFieldAccessorInterface() = default;

    virtual std::unique_ptr<DataFieldAccessorInterface> clone() const = 0;

    virtual DataType value()                           = 0;
    virtual void     setValue( const DataType& value ) = 0;

    virtual std::optional<DataType> defaultValue() const { return std::nullopt; }
    virtual void                    setDefaultValue( const DataType& value ) {}
};

template <class DataType>
class DataFieldDirectStorageAccessor : public DataFieldAccessorInterface<DataType>
{
public:
    DataFieldDirectStorageAccessor() = default;
    DataFieldDirectStorageAccessor( const DataType& defaultValue )
        : m_value( defaultValue )
        , m_defaultValue( defaultValue )

    {
    }

    std::unique_ptr<DataFieldAccessorInterface<DataType>> clone() const override
    {
        std::unique_ptr<DataFieldAccessorInterface<DataType>> copy;
        if ( m_defaultValue.has_value() )
            copy = std::make_unique<DataFieldDirectStorageAccessor<DataType>>( *m_defaultValue );
        else
            copy = std::make_unique<DataFieldDirectStorageAccessor<DataType>>();

        copy->setValue( m_value );
        return copy;
    }

    DataType value() override { return m_value; };

    void                    setValue( const DataType& value ) override { m_value = value; }
    std::optional<DataType> defaultValue() const override { return m_defaultValue; }
    void                    setDefaultValue( const DataType& value ) override { m_defaultValue = value; }

private:
    DataType                m_value;
    std::optional<DataType> m_defaultValue;
};

} // namespace caf
