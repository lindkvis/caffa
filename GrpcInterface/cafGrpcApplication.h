//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3d Radar AS
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
#pragma once

#include "cafApplication.h"

#include <memory>

namespace caffa::rpc
{
class Server;

class Application : public caffa::Application
{
public:
    Application( const unsigned int& capabilities );
    Application( const AppInfo::AppCapability& capability );

    static Application* instance();
    size_t              packageByteSize() const;
    void                setPackageByteSize( size_t packageByteSize );

    static std::string readKeyAndCertificate( const std::string& path );

private:
    size_t m_packageByteSize;
};
} // namespace caffa::rpc
