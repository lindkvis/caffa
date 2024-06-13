// ##################################################################################################
//
//    Caffa
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

#include "cafNotNull.h"
#include "cafRpcApplication.h"
#include "cafSession.h"

#include <memory>

namespace caffa
{

namespace rpc
{

    class ServerApplication : public RpcApplication
    {
    public:
        ServerApplication( unsigned capability );
        ServerApplication( AppInfo::AppCapability capability );

        static ServerApplication* instance();

        virtual int  portNumber() const     = 0;
        virtual void run()                  = 0;
        virtual void quit()                 = 0;
        virtual bool running() const        = 0;
        virtual void waitUntilReady() const = 0;

        virtual std::shared_ptr<Document> document( const std::string& documentId, const caffa::Session* session ) = 0;
        virtual std::shared_ptr<const Document>            document( const std::string&    documentId,
                                                                     const caffa::Session* session ) const         = 0;
        virtual std::list<std::shared_ptr<Document>>       documents( const caffa::Session* session )              = 0;
        virtual std::list<std::shared_ptr<const Document>> documents( const caffa::Session* session ) const        = 0;

        virtual std::list<std::shared_ptr<caffa::Document>> defaultDocuments() const = 0;

        bool requiresValidSession() const;
        void setRequiresValidSession( bool requiresValidSession );

        virtual bool                          readyForSession( caffa::Session::Type type ) const             = 0;
        virtual caffa::SessionMaintainer      createSession( caffa::Session::Type type )                     = 0;
        virtual bool                          hasActiveSessions() const                                      = 0;
        virtual caffa::SessionMaintainer      getExistingSession( const std::string& sessionUuid )           = 0;
        virtual caffa::ConstSessionMaintainer getExistingSession( const std::string& sessionUuid ) const     = 0;
        virtual void changeSession( caffa::not_null<caffa::Session*> session, caffa::Session::Type newType ) = 0;
        virtual void destroySession( const std::string& sessionUuid )                                        = 0;

    protected:
        virtual void onStartup() {}
        virtual void onShutdown() {}

    private:
        bool m_requiresValidSession;
    };
} // namespace rpc
} // namespace caffa