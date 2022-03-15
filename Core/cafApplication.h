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

/**
 * @brief Main Caffa namespace
 *
 */
namespace caffa
{
class Document;

/**
 * @brief Basic Application Information.
 *
 */
struct AppInfo
{
    /**
     * @brief Application capability
     * Defines what type of application it is. These flags can be combined. I.e. a Console GRPC-Server or GUI
     * Grpc-client.
     *
     */
    enum class AppCapability : unsigned int
    {
        CONSOLE     = 0x00, ///< Console Application
        GRPC_SERVER = 0x01, ///< gRPC server
        GRPC_CLIENT = 0x02, ///< gRPC client
        GUI         = 0x04, ///< GUI application
        WEB         = 0x08 ///< Web server
    };

    /**
     * @brief The name of the application
     *
     */
    std::string name;
    /**
     * @brief Major version number
     *
     */
    int majorVersion;
    /**
     * @brief Minor version number
     *
     */
    int minorVersion;
    /**
     * @brief Patch version number
     *
     */
    int patchVersion;
    /**
     * @brief Application type. Can be CONSOLE, GRPC_SERVER, GRPC_CLIENT, GUI or WEB.
     *
     */
    unsigned int appType;

    /**
     * @brief Check if the application has the specified capability
     *
     * @param typeToCheck
     * @return true
     * @return false
     */
    bool hasCapability( AppCapability typeToCheck ) const
    {
        return ( appType & static_cast<unsigned int>( typeToCheck ) ) != 0u;
    }

    /**
     * @brief Construct a full X.Y.Z version string with major, minor and patch version.
     *
     * @return std::string
     */
    std::string version_string() const
    {
        return std::to_string( majorVersion ) + "." + std::to_string( minorVersion ) + "." + std::to_string( patchVersion );
    }
};

class Application
{
public:
    Application( const unsigned int& capabilities );
    Application( const AppInfo::AppCapability& capability );
    virtual ~Application();

    virtual std::string name() const = 0;
    bool                hasCapability( AppInfo::AppCapability typeToCheck ) const;
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
