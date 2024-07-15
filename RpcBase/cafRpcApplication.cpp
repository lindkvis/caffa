// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- 3d Radar AS
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
#include "cafRpcApplication.h"

#include "cafLogger.h"

#include <fstream>
#include <sstream>

using namespace caffa::rpc;

RpcApplication::RpcApplication( unsigned int capabilities )
    : caffa::Application( capabilities )
{
}

RpcApplication::RpcApplication( AppInfo::AppCapability capability )
    : caffa::Application( capability )
{
}

RpcApplication* RpcApplication::instance()
{
    caffa::Application* appInstance = caffa::Application::instance();
    return dynamic_cast<RpcApplication*>( appInstance );
}

std::string RpcApplication::readKeyOrCertificate( const std::string& path )
{
    std::ifstream stream( path );
    if ( !stream.good() )
    {
        CAFFA_CRITICAL( "Failed to open file: " << path );
    }
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

namespace caffa
{
nlohmann::json appJsonSchema()
{
    auto appInfoSchema = nlohmann::json::object();

    auto properties             = nlohmann::json::object();
    properties["name"]          = { { "type", "string" } };
    properties["type"]          = { { "type", "integer" }, { "format", "int32" } };
    properties["major_version"] = { { "type", "integer" }, { "format", "int32" } };
    properties["minor_version"] = { { "type", "integer" }, { "format", "int32" } };
    properties["patch_version"] = { { "type", "integer" }, { "format", "int32" } };
    properties["description"]   = { { "type", "string" } };
    properties["contact_email"] = { { "type", "string" } };

    appInfoSchema["type"]       = "object";
    appInfoSchema["properties"] = properties;

    return appInfoSchema;
}

void to_json( nlohmann::json& jsonValue, const caffa::AppInfo& appInfo )
{
    jsonValue                  = nlohmann::json::object();
    jsonValue["name"]          = appInfo.name;
    jsonValue["type"]          = appInfo.appType;
    jsonValue["major_version"] = appInfo.majorVersion;
    jsonValue["minor_version"] = appInfo.minorVersion;
    jsonValue["patch_version"] = appInfo.patchVersion;
    jsonValue["description"]   = appInfo.description;
    jsonValue["contact_email"] = appInfo.contactEmail;
}

void from_json( const nlohmann::json& jsonValue, caffa::AppInfo& appInfo )
{
    appInfo.name         = jsonValue["name"].get<std::string>();
    appInfo.appType      = jsonValue["type"].get<unsigned>();
    appInfo.majorVersion = jsonValue["major_version"].get<int>();
    appInfo.minorVersion = jsonValue["minor_version"].get<int>();
    appInfo.patchVersion = jsonValue["patch_version"].get<int>();
    appInfo.description  = jsonValue["description"].get<std::string>();
    appInfo.contactEmail = jsonValue["contact_email"].get<std::string>();
}
} // namespace caffa