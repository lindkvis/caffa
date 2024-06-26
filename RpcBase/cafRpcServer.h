// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2020-2021 Gaute Lindkvist
//    Copyright (C) 2021- Kontur AS
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

#include <string>

namespace caffa
{
class Object;
class Session;
} // namespace caffa

namespace caffa::rpc
{
class Server
{
public:
    Server()          = default;
    virtual ~Server() = default;

    static caffa::Object* findCafObjectFromJsonObject( const caffa::Session* session, const std::string& jsonObject );
    static caffa::Object* findCafObjectFromScriptNameAndUuid( const caffa::Session* session,
                                                              const std::string&    scriptClassName,
                                                              const std::string&    objectUuid );

    virtual void run()           = 0;
    virtual void quit()          = 0;
    virtual bool running() const = 0;

private:
    Server( const Server& ) = delete;
};
} // namespace caffa::rpc
