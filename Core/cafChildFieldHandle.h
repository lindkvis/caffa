// ##################################################################################################
//
//    Copyright (C) 2023- Kontur AS
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

#include "cafFieldHandle.h"
#include "cafObjectHandle.h"
#include "cafVisitor.h"

namespace caffa
{
class ChildFieldAccessor;
class ObjectHandle;

template <typename DataTypePtr>
concept is_pointer = std::is_pointer<DataTypePtr>::value;

class ChildFieldBaseHandle : public FieldHandle
{
public:
    /**
     * @brief Get the class keyword of the contained child(ren)
     *
     */
    virtual constexpr std::string_view childClassKeyword() const = 0;

    virtual std::vector<ObjectHandle::Ptr>      childObjects()       = 0;
    virtual std::vector<ObjectHandle::ConstPtr> childObjects() const = 0;
    void accept( Inspector* visitor ) const override { visitor->visitChildField( this ); }
    void accept( Editor* visitor ) override { visitor->visitChildField( this ); }
};

class ChildFieldHandle : public ChildFieldBaseHandle
{
public:
    virtual void setAccessor( std::unique_ptr<ChildFieldAccessor> accessor ) = 0;
    virtual void clear()                                                     = 0;
};
} // namespace caffa