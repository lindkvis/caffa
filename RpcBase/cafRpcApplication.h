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
#pragma once

#include "cafApplication.h"
#include "cafJsonDefinitions.h"

#include <string>

namespace caffa
{

namespace rpc
{
    class RpcApplication : public Application
    {
    public:
        explicit RpcApplication( unsigned int capabilities );
        explicit RpcApplication( AppInfo::AppCapability capability );

        static RpcApplication* instance();

        static std::string readKeyOrCertificate( const std::string& path );
    };
} // namespace rpc

json::object appJsonSchema();

} // namespace caffa
