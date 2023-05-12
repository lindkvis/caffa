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

namespace caffa
{
class ChildFieldBaseHandle;
class ObjectHandle;
class FieldHandle;

/**
 * A visitor which is only allowed to inspect objects
 */
class Inspector
{
public:
    virtual void visitObject( const ObjectHandle* object )            = 0;
    virtual void visitChildField( const ChildFieldBaseHandle* field ) = 0;
    virtual void visitField( const FieldHandle* field )               = 0;
};

/**
 * A visitor which is allowed to edit objects
 */
class Editor
{
public:
    virtual void visitObject( ObjectHandle* object )            = 0;
    virtual void visitChildField( ChildFieldBaseHandle* field ) = 0;
    virtual void visitField( FieldHandle* field )               = 0;
};

} // namespace caffa
