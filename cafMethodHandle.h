// ##################################################################################################
//
//    Caffa
//    Copyright (C) Kontur As
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
#pragma once

#include <string>
#include <vector>

namespace caffa
{
class ObjectHandle;

class MethodHandle
{
public:
    MethodHandle()
        : m_isConst( false )
    {
    }
    virtual ~MethodHandle() = default;

    [[nodiscard]] std::string keyword() const { return m_name; }
    void setArgumentNames( const std::vector<std::string>& argumentNames ) { m_argumentNames = argumentNames; }
    [[nodiscard]] const std::vector<std::string>& argumentNames() const { return m_argumentNames; }

    [[nodiscard]] bool               isConst() const { return m_isConst; }
    void                             setConst( bool isConst ) { m_isConst = isConst; }
    [[nodiscard]] const std::string& documentation() const { return m_documentation; }
    void setDocumentation( const std::string& documentation ) { m_documentation = documentation; }

private:
    friend class ObjectHandle;
    void setName( const std::string& name ) { m_name = name; }

    std::string              m_name;
    std::vector<std::string> m_argumentNames;
    bool                     m_isConst;
    std::string              m_documentation;
};
} // namespace caffa