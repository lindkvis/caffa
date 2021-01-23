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
#include "cafPdmScriptIOMessages.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::addWarning( const std::string& message )
{
    m_messages.push_back(
        std::make_pair( MESSAGE_WARNING, "Line " + std::to_string( m_currentLineNumber ) + ": " + message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::addError( const std::string& message )
{
    m_messages.push_back(
        std::make_pair( MESSAGE_ERROR, "Line " + std::to_string( m_currentLineNumber ) + ": " + message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::skipWhiteSpaceWithLineNumberCount( std::istream& inputStream )
{
    while ( !inputStream.eof() )
    {
        char ch = readCharWithLineNumberCount( inputStream );
        if ( !std::isspace(ch) )
        {
            inputStream.unget();
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
char PdmScriptIOMessages::readCharWithLineNumberCount( std::istream& inputStream )
{
    char ch;
    inputStream >> ch;
    if ( ch == char( '\n' ) )
    {
        m_currentLineNumber++;
    }
    return ch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
char PdmScriptIOMessages::peekNextChar( std::istream& inputStream )
{
    char ch;
    if ( !inputStream.eof() )
    {
        ch = inputStream.peek();
    }
    return ch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmScriptIOMessages::skipLineWithLineNumberCount( std::istream& inputStream )
{
    std::string dummy;
    std::getline( inputStream, dummy );
    m_currentLineNumber++;
}
