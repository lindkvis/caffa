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

#include "cafPdmDocument.h"

#include <QFile>
#include <QXmlStreamReader>

namespace caf
{
CAF_PDM_SOURCE_INIT( PdmDocument, "PdmDocument" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmDocument::PdmDocument()
{
    CAF_PDM_InitFieldNoDefault( &fileName, "DocumentFileName", "File Name", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::readFile()
{
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

    readFile( &file );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::readFile(
    QIODevice*                                       file,
    caf::ObjectIoCapability::IoParameters::IoType ioType /*= ObjectIoCapability::IoParameters::IoType::XML */ )
{
    ObjectIoCapability::IoParameters params = { ioType, file };
    ObjectIoCapability::readFile( params );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmDocument::writeFile()
{
    QFile inputFile( fileName );
    if ( !inputFile.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;

    writeFile( &inputFile );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::writeFile( QIODevice* file, caf::ObjectIoCapability::IoParameters::IoType ioType )
{
    ObjectIoCapability::IoParameters params = { ioType, file };

    ObjectIoCapability::writeFile( params );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::updateUiIconStateRecursively( ObjectHandle* object )
{
    if ( object == nullptr ) return;
    std::vector<FieldHandle*> fields;
    object->fields( fields );

    std::vector<ObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        if ( fields[fIdx] ) fields[fIdx]->childObjects( &children );
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        PdmDocument::updateUiIconStateRecursively( children[cIdx] );
    }

    ObjectUiCapability* uiObjectHandle = uiObj( object );
    if ( uiObjectHandle )
    {
        uiObjectHandle->updateUiIconFromToggleField();
    }
}

} // End of namespace caf
