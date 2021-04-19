//##################################################################################################
//
//   Caffa Toolkit
//   Copyright (C) Gaute Lindkvist
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

#include <list>
#include <set>
#include <string>

namespace caffa
{
class Document;

enum class AppCapability : unsigned int
{
    CONSOLE     = 0x00,
    GRPC_SERVER = 0x01,
    GRPC_CLIENT = 0x02,
    GUI         = 0x04,
    WEB         = 0x08
};

struct AppInfo
{
    std::string  name;
    int          majorVersion;
    int          minorVersion;
    int          patchVersion;
    unsigned int appType;

    bool hasCapability( AppCapability typeToCheck ) const
    {
        return ( appType & static_cast<unsigned int>( typeToCheck ) ) != 0u;
    }
};

class Application
{
public:
    Application( const unsigned int& capabilities );
    Application( const AppCapability& capability );
    virtual ~Application();

    virtual std::string name() const = 0;
    bool                hasCapability( AppCapability typeToCheck ) const;
    AppInfo             appInfo() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;
    virtual int patchVersion() const = 0;

    static Application* instance();
    static void         registerInstance( Application* instance );

private:
    static Application* s_instance;

    unsigned int m_capabilities;
};

} // namespace caffa
