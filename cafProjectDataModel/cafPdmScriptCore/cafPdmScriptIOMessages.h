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
#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace caf
{
class PdmScriptIOMessages
{
public:
    PdmScriptIOMessages()
        : m_currentLineNumber( 1 )
    {
    }

    enum MessageType
    {
        MESSAGE_WARNING,
        MESSAGE_ERROR
    };

    void addWarning( const std::string& message );
    void addError( const std::string& message );

    void skipWhiteSpaceWithLineNumberCount( std::istream& inputStream );
    void skipLineWithLineNumberCount( std::istream& inputStream );

    char readCharWithLineNumberCount( std::istream& inputStream );
    char peekNextChar( std::istream& inputStream );

    std::string                                      currentCommand;
    std::string                                      currentArgument;
    std::vector<std::pair<MessageType, std::string>> m_messages;

private:
    int m_currentLineNumber;
};

} // namespace caf