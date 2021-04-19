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

#include "cafDocument.h"
#include "cafFieldScriptingCapability.h"

#include <fstream>

namespace caffa
{
CAFFA_SOURCE_INIT( Document, "Document", "Object" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Document::Document()
{
    assignUiInfo( "Document", "", "Basic Document", "" );
    initField( fileName, "DocumentFileName" ).withScripting();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Document::read( ObjectIoCapability::IoType ioType /*= ObjectIoCapability::IoType::JSON */ )
{
    return ObjectIoCapability::readFile( fileName, ioType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Document::write( ObjectIoCapability::IoType ioType /*= ObjectIoCapability::IoType::JSON */ )
{
    return ObjectIoCapability::writeFile( fileName, ioType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Document::updateUiIconStateRecursively( ObjectHandle* object )
{
    if ( object == nullptr ) return;

    for ( auto child : object->children() )
    {
        Document::updateUiIconStateRecursively( child );
    }

    ObjectUiCapability* uiObjectHandle = uiObj( object );
    if ( uiObjectHandle )
    {
        uiObjectHandle->updateUiIconFromToggleField();
    }
}

} // End of namespace caffa
