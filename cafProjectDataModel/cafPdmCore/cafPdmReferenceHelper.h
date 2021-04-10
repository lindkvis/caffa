//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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
//##################################################################################################

#pragma once

#include "cafObjectHandle.h"
#include "cafPointer.h"

#include <list>
#include <string>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmReferenceHelper
{
public:
    static ObjectHandle* findRoot( ObjectHandle* obj );
    static ObjectHandle* findRoot( FieldHandle* field );

    static std::string   referenceFromRootToField( ObjectHandle* root, FieldHandle* field );
    static std::string   referenceFromRootToObject( ObjectHandle* root, ObjectHandle* obj );
    static ObjectHandle* objectFromReference( ObjectHandle* root, const std::string& reference );
    static FieldHandle*  fieldFromReference( ObjectHandle* root, const std::string& reference );

    static std::string   referenceFromFieldToObject( FieldHandle* fromField, ObjectHandle* toObj );
    static ObjectHandle* objectFromFieldReference( FieldHandle* fromField, const std::string& reference );

private:
    static std::list<std::string> referenceFromRootToObjectAsStringList( ObjectHandle* root, ObjectHandle* obj );
    static ObjectHandle* objectFromReferenceStringList( ObjectHandle* root, const std::list<std::string>& reference );

    static FieldHandle* findField( ObjectHandle* object, const std::string& fieldKeyword );
};

} // end namespace caf
