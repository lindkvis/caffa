// ##################################################################################################
//
//    CAFFA
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

namespace caffa
{
class ObjectHandle;

class MethodHandle
{
public:
    enum class Type
    {
        READ_WRITE = 0,
        READ_ONLY
    };

    std::string name() const { return m_name; }
    Type        type() const { return m_type; }

    virtual std::string execute( const std::string& jsonArguments ) const = 0;

private:
    friend class ObjectHandle;
    void setName( const std::string name ) { m_name = name; }
    void setType( Type type ) { m_type = type; }

    std::string m_name;
    Type        m_type;
};
} // namespace caffa