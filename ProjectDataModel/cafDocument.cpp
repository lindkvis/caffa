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
Document::Document( const std::string& id )
{
    assignUiInfo( "Document", "Basic Document", "" );
    initField( m_id, "id" ).withScripting().withDefault( id ).withUi( "Document ID", "Unique document ID" );
    initField( m_fileName, "fileName" ).withScripting().withUi( "Document File Name", "File Name for serialisation" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Document::id() const
{
    return m_id.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Document::fileName() const
{
    return m_fileName.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Document::setId( const std::string& id )
{
    m_id.setValue( id );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Document::setFileName( const std::string& fileName )
{
    m_fileName.setValue( fileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Document::read( ObjectIoCapability::IoType ioType /*= ObjectIoCapability::IoType::JSON */ )
{
    return ObjectIoCapability::readFile( m_fileName, ioType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Document::write( ObjectIoCapability::IoType ioType /*= ObjectIoCapability::IoType::JSON */ )
{
    return ObjectIoCapability::writeFile( m_fileName, ioType );
}

} // End of namespace caffa
