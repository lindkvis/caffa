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

#include "cafVisitor.h"

#include "cafChildArrayFieldHandle.h"
#include "cafChildFieldHandle.h"
#include "cafObjectHandle.h"
#include "cafTypedField.h"

using namespace caffa;

void Inspector::visit( const ObjectHandle* object )
{
    visitObject( object );

    for ( auto field : object->fields() )
    {
        if ( field->isReadable() )
        {
            field->accept( this );
        }
    }

    leaveObject( object );
}

void Inspector::visit( const ChildFieldBaseHandle* field )
{
    visitField( field );

    for ( auto object : field->childObjects() )
    {
        object->accept( this );
    }

    leaveField( field );
}

void Inspector::visit( const DataField* field )
{
    visitField( field );
    leaveField( field );
}

void Editor::visit( ObjectHandle* object )
{
    visitObject( object );

    for ( auto field : object->fields() )
    {
        if ( field->isWritable() )
        {
            field->accept( this );
        }
    }
    leaveObject( object );
}

void Editor::visit( ChildFieldBaseHandle* field )
{
    visitField( field );

    for ( auto object : field->childObjects() )
    {
        object->accept( this );
    }
    leaveField( field );
}

void Editor::visit( DataField* field )
{
    visitField( field );
    leaveField( field );
}
