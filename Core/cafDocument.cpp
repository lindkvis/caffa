// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013-2020 Ceetron Solutions AS
//    Copyright (C) 2022- Kontur AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
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

#include "cafDocument.h"
#include "cafFieldScriptingCapability.h"
#include "cafObjectPerformer.h"

#include <fstream>

namespace caffa
{
CAFFA_SOURCE_INIT( Document )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Document::Document( const std::string& id )
{
    initField( m_id, "id" ).withScripting().withDefault( id ).withDoc( "A unique document ID" );
    initField( m_fileName, "fileName" ).withScripting().withDoc( "The filename of the document if saved to disk" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Document::~Document() noexcept
{
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
bool Document::read()
{
    std::ifstream inStream( m_fileName );
    if ( !inStream.good() )
    {
        CAFFA_ERROR( "Could not open file for reading: " << m_fileName() );
        return false;
    }

    JsonSerializer serializer;

    try
    {
        serializer.readStream( this, inStream );

        ObjectPerformer<> performer( []( ObjectHandle* object ) { object->initAfterRead(); } );
        performer.visitObject( this );
    }
    catch ( std::runtime_error& err )
    {
        CAFFA_ERROR( err.what() );
        return false;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Generic object reading error" );
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Document::write()
{
    std::ofstream outStream( m_fileName );

    if ( !outStream.good() )
    {
        CAFFA_ERROR( "Could not open file for writing: " << m_fileName() );
        return false;
    }

    try
    {
        JsonSerializer serializer;
        serializer.setSerializeDataTypes( false ).setSerializeUuids( false );
        serializer.writeStream( this, outStream );
    }
    catch ( std::runtime_error& err )
    {
        CAFFA_ERROR( err.what() );
        return false;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Generic object writing error" );
        return false;
    }
    return true;
}

} // End of namespace caffa
