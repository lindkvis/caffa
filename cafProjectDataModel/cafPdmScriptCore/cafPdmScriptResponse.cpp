//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cafPdmScriptResponse.h"
#include "cafStringTools.h"

#include <regex>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmScriptResponse::PdmScriptResponse( Status status, const std::string& message )
    : m_status( COMMAND_OK )
{
    updateStatus( status, message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmScriptResponse::PdmScriptResponse( Object* ok_result )
    : m_status( COMMAND_OK )
    , m_result( ok_result )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmScriptResponse::Status PdmScriptResponse::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
/// The resulting message is sent in HTTP metadata and must not have any newlines.
//--------------------------------------------------------------------------------------------------
std::string PdmScriptResponse::sanitizedResponseMessage() const
{
    std::string completeMessage = caf::StringTools::join(m_messages.begin(), m_messages.end(), ";;" );
    std::regex regex( "\n" );
    return std::regex_replace( completeMessage, regex, ";;" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::string> PdmScriptResponse::messages() const
{
    return m_messages;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object* PdmScriptResponse::result() const
{
    return m_result.get();
}

//--------------------------------------------------------------------------------------------------
/// Takes ownership of the result object
//--------------------------------------------------------------------------------------------------
void PdmScriptResponse::setResult( Object* result )
{
    m_result.reset( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptResponse::updateStatus( Status status, const std::string& message )
{
    m_status = std::max( m_status, status );
    if ( !message.empty() )
    {
        m_messages.push_back( statusLabel( status ) + ": " + message );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string PdmScriptResponse::statusLabel( Status status )
{
    switch ( status )
    {
        case COMMAND_WARNING:
            return "Warning";
        case COMMAND_ERROR:
            return "Error";
        default:
            return "";
    }
}
