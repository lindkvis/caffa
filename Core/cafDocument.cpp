// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013-2022 Ceetron Solutions AS
//    Copyright (C) Changes 2022- Kontur AS
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

#include <fstream>

namespace caffa
{
CAFFA_SOURCE_INIT( Document )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Document::Document( const std::string& id )
{
    initField( m_id, "id" ).withScripting( true, false ).withDefault( id ).withDoc( "A unique document ID" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Document::~Document() noexcept = default;

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
void Document::setId( const std::string& id )
{
    m_id.setValue( id );
}

} // End of namespace caffa
