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
class MethodHandle;
class ObjectHandle;

class MethodAccessorInterface
{
public:
    MethodAccessorInterface( const ObjectHandle* selfHandle, const MethodHandle* methodHandle )
        : m_selfHandle( selfHandle )
        , m_methodHandle( methodHandle )
    {
    }
    virtual std::string execute( const std::string& jsonArgumentsString ) const = 0;

protected:
    const ObjectHandle* m_selfHandle;
    const MethodHandle* m_methodHandle;
};

class MethodHandle
{
public:
    enum class Type
    {
        READ_WRITE = 0,
        READ_ONLY
    };

    MethodHandle()
        : m_documentation( "A generic object method" )
    {
    }

    std::string name() const { return m_name; }
    void        setArgumentNames( const std::vector<std::string>& argumentNames ) { m_argumentNames = argumentNames; }
    const std::vector<std::string>& argumentNames() const { return m_argumentNames; }

    Type               type() const { return m_type; }
    const std::string& documentation() const { return m_documentation; }
    void               setDocumentation( const std::string& documentation ) { m_documentation = documentation; }

    virtual std::string execute( const std::string& jsonArgumentsString ) const = 0;
    virtual std::string schema() const                                          = 0;

    MethodAccessorInterface* accessor() const { return m_accessor.get(); }
    void setAccessor( std::unique_ptr<MethodAccessorInterface> accessor ) { m_accessor = std::move( accessor ); }

    virtual std::vector<std::string> dependencies() const = 0;

private:
    friend class ObjectHandle;
    void setName( const std::string& name ) { m_name = name; }
    void setType( Type type ) { m_type = type; }

    std::string              m_name;
    std::vector<std::string> m_argumentNames;
    Type                     m_type;
    std::string              m_documentation;

    std::unique_ptr<MethodAccessorInterface> m_accessor;
};
} // namespace caffa