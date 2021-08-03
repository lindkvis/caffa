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

#include "cafAssert.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

namespace caffa
{
class FieldHandle;
class ObjectHandle;

class ChildArrayFieldAccessor
{
public:
    ChildArrayFieldAccessor( FieldHandle* field )
        : m_field( field )
    {
    }
    virtual ~ChildArrayFieldAccessor()                               = default;
    virtual size_t                                     size() const  = 0;
    virtual std::vector<std::unique_ptr<ObjectHandle>> clear()       = 0;
    virtual std::vector<ObjectHandle*>                 value() const = 0;

    virtual ObjectHandle*                 at( size_t index ) const                                      = 0;
    virtual void                          insert( size_t index, std::unique_ptr<ObjectHandle> pointer ) = 0;
    virtual void                          push_back( std::unique_ptr<ObjectHandle> pointer )            = 0;
    virtual size_t                        index( const ObjectHandle* pointer ) const                    = 0;
    virtual std::unique_ptr<ObjectHandle> remove( size_t index )                                        = 0;

protected:
    FieldHandle* m_field;
};

class ChildArrayFieldDirectStorageAccessor : public ChildArrayFieldAccessor
{
public:
    ChildArrayFieldDirectStorageAccessor( FieldHandle* field );
    ~ChildArrayFieldDirectStorageAccessor() override;

    size_t                                     size() const override;
    std::vector<std::unique_ptr<ObjectHandle>> clear() override;
    std::vector<ObjectHandle*>                 value() const override;
    ObjectHandle*                              at( size_t index ) const override;
    void                                       insert( size_t index, std::unique_ptr<ObjectHandle> pointer ) override;
    void                                       push_back( std::unique_ptr<ObjectHandle> pointer ) override;
    size_t                                     index( const ObjectHandle* object ) const override;
    std::unique_ptr<ObjectHandle>              remove( size_t index ) override;

private:
    std::vector<std::unique_ptr<ObjectHandle>> m_pointers;
};

} // namespace caffa
