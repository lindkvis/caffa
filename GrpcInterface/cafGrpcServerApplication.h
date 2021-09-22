//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar AS
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

#include "cafGrpcApplication.h"
#include "cafGrpcServer.h"

#include <memory>

namespace caffa::rpc
{
class ServerApplication : public Application
{
public:
    ServerApplication( int portNumber );
    static ServerApplication* instance();

    void run();
    bool running() const;
    void quit();

    virtual Document*                  document( const std::string& documentId )       = 0;
    virtual const Document*            document( const std::string& documentId ) const = 0;
    virtual std::list<Document*>       documents()                                     = 0;
    virtual std::list<const Document*> documents() const                               = 0;

private:
    virtual void onStartup()  = 0;
    virtual void onShutdown() = 0;

private:
    std::unique_ptr<Server> m_server;
};
} // namespace caffa::rpc
