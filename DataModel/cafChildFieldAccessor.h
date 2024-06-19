// ##################################################################################################
//
//    Caffa
//    Copyright (C) 3D-Radar AS
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

#include "cafObjectHandle.h"

#include <memory>
#include <optional>

namespace caffa
{
class FieldHandle;

class ChildFieldAccessor
{
public:
    ChildFieldAccessor( FieldHandle* field )
        : m_field( field )
    {
    }
    virtual ~ChildFieldAccessor()                                                                 = default;
    virtual std::shared_ptr<ObjectHandle>       object()                                          = 0;
    virtual std::shared_ptr<const ObjectHandle> object() const                                    = 0;
    virtual void                                setObject( std::shared_ptr<ObjectHandle> object ) = 0;
    virtual void                                clear()                                           = 0;

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

protected:
    FieldHandle* m_field;
};

class ChildFieldDirectStorageAccessor : public ChildFieldAccessor
{
public:
    ChildFieldDirectStorageAccessor( FieldHandle* field )
        : ChildFieldAccessor( field )
    {
    }
    ~ChildFieldDirectStorageAccessor() override = default;
    std::shared_ptr<ObjectHandle>       object() override { return m_object; }
    std::shared_ptr<const ObjectHandle> object() const override { return m_object; }
    void setObject( std::shared_ptr<ObjectHandle> object ) override { m_object = object; }
    void clear() override { m_object.reset(); }

    bool hasGetter() const override { return true; }
    bool hasSetter() const override { return true; }

private:
    std::shared_ptr<ObjectHandle> m_object;
};

} // namespace caffa
