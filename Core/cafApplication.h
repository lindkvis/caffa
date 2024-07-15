// ##################################################################################################
//
//    Caffa Toolkit
//    Copyright (C) Kontur AS
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
#pragma once

#include <string>

/**
 * @brief Main Caffa namespace
 *
 */
namespace caffa
{

/**
 * @brief Basic Application Information.
 *
 */
struct AppInfo
{
    /**
     * @brief Application capability
     * Defines what type of application it is. These flags can be combined. I.e. a Console Server or GUI
     * client.
     *
     */
    enum class AppCapability : unsigned int
    {
        SERVER,
        CLIENT
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
     * @brief Application type. Can be CONSOLE, SERVER, CLIENT, GUI
     *
     */
    unsigned int appType;

    /**
     * @brief Application description.
     */
    std::string description;

    /**
     * @brief Contact email
     */
    std::string contactEmail;

    /**
     * @brief Check if the application has the specified capability
     *
     * @param typeToCheck
     * @return true
     * @return false
     */
    [[nodiscard]] bool hasCapability( AppCapability typeToCheck ) const
    {
        return ( appType & static_cast<unsigned int>( typeToCheck ) ) != 0u;
    }

    /**
     * @brief Construct a full X.Y.Z version string with major, minor and patch version.
     *
     * @return std::string
     */
    [[nodiscard]] std::string version_string() const
    {
        return std::to_string( majorVersion ) + "." + std::to_string( minorVersion ) + "." + std::to_string( patchVersion );
    }
};

class Application
{
public:
    explicit Application( unsigned int capabilities );
    explicit Application( AppInfo::AppCapability capability );
    virtual ~Application();

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] bool                hasCapability( AppInfo::AppCapability typeToCheck ) const;
    [[nodiscard]] AppInfo             appInfo() const;

    [[nodiscard]] virtual int         majorVersion() const = 0;
    [[nodiscard]] virtual int         minorVersion() const = 0;
    [[nodiscard]] virtual int         patchVersion() const = 0;
    [[nodiscard]] virtual std::string description() const  = 0;
    [[nodiscard]] virtual std::string contactEmail() const = 0;

    static Application* instance();
    static void         registerInstance( Application* instance );

    static void assertCapability( AppInfo::AppCapability typeToAssert );

private:
    static Application* s_instance;

    unsigned int m_capabilities;
};

} // namespace caffa
