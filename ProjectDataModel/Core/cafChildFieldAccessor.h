//##################################################################################################
//
//   Caffa
//   Copyright (C) 3D-Radar AS
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
class FieldHandle;

template <class DataType>
class ChildFieldAccessor
{
public:
    ChildFieldAccessor( FieldHandle* field )
        : m_field( field )
    {
    }
    virtual ~ChildFieldAccessor()                                                 = default;
    virtual DataType*                 value() const                               = 0;
    virtual void                      setValue( std::unique_ptr<DataType> value ) = 0;
    virtual std::unique_ptr<DataType> remove( ObjectHandle* object )              = 0;

protected:
    FieldHandle* m_field;
};

template <class DataType>
class ChildFieldDirectStorageAccessor : public ChildFieldAccessor<DataType>
{
public:
    ChildFieldDirectStorageAccessor( FieldHandle* field )
        : ChildFieldAccessor<DataType>( field )
    {
    }
    ~ChildFieldDirectStorageAccessor() override = default;
    DataType* value() const override { return m_value.get(); };
    void      setValue( std::unique_ptr<DataType> value ) override
    {
        if ( m_value )
        {
            m_value->detachFromParentField();
        }
        value->setAsParentField( this->m_field );
        m_value = std::move( value );
    }
    std::unique_ptr<DataType> remove( ObjectHandle* object ) override
    {
        if ( m_value.get() == object )
        {
            return std::move( m_value );
        }
        return nullptr;
    }

private:
    std::unique_ptr<DataType> m_value;
};

} // namespace caffa
