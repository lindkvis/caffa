// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
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

#include "cafRpcApplication.h"

#include <memory>
#include <string>

namespace caffa::rpc
{
class RestClient;

class RestClientApplication : public RpcApplication
{
public:
    RestClientApplication( const std::string& hostname,
                           int                portNumber,
                           const std::string& username = "",
                           const std::string& password = "" );
    static RestClientApplication* instance();

    [[nodiscard]] RestClient*       client();
    [[nodiscard]] const RestClient* client() const;

private:
    std::unique_ptr<RestClient> m_client;
};
} // namespace caffa::rpc
