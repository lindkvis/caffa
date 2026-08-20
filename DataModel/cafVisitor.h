// ##################################################################################################
//
//    CAFFA
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

#include <cstddef>

namespace caffa
{
class ChildFieldBaseHandle;
class DataField;
class FieldHandle;
class ObjectHandle;

/**
 * A visitor which is only allowed to inspect objects
 */
class Inspector
{
public:
    virtual ~Inspector() = default;

    virtual void visit( const std::shared_ptr<const ObjectHandle>& object ) = 0;
    virtual void visit( const ChildFieldBaseHandle* field )                 = 0;
    virtual void visit( const DataField* field )                            = 0;
};

/**
 * A visitor which is allowed to edit objects
 */
class Editor
{
public:
    virtual ~Editor() = default;

    virtual void visit( const std::shared_ptr<ObjectHandle>& object ) = 0;
    virtual void visit( ChildFieldBaseHandle* field )                 = 0;
    virtual void visit( DataField* field )                            = 0;
};

} // namespace caffa
