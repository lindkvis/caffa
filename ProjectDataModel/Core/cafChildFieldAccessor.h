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
class ObjectHandle;

class ChildFieldAccessor
{
public:
    ChildFieldAccessor( FieldHandle* field )
        : m_field( field )
    {
    }
    virtual ~ChildFieldAccessor()                                                         = default;
    virtual ObjectHandle*                 value() const                                   = 0;
    virtual void                          setValue( std::unique_ptr<ObjectHandle> value ) = 0;
    virtual std::unique_ptr<ObjectHandle> clear()                                         = 0;

protected:
    FieldHandle* m_field;
};

class ChildFieldDirectStorageAccessor : public ChildFieldAccessor
{
public:
    ChildFieldDirectStorageAccessor( FieldHandle* field );
    ~ChildFieldDirectStorageAccessor() override = default;
    ObjectHandle*                 value() const override;
    void                          setValue( std::unique_ptr<ObjectHandle> value ) override;
    std::unique_ptr<ObjectHandle> clear() override;

private:
    std::unique_ptr<ObjectHandle> m_value;
};

} // namespace caffa