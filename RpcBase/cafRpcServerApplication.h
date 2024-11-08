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

#include <list>
#include <memory>

namespace caffa
{
class Document;

namespace rpc
{

    class ServerApplication : public RpcApplication
    {
    public:
        explicit ServerApplication( unsigned capability );
        explicit ServerApplication( AppInfo::AppCapability capability );

        static ServerApplication* instance();

        [[nodiscard]] virtual int  portNumber() const = 0;
        virtual void               run()              = 0;
        virtual void               quit()             = 0;
        [[nodiscard]] virtual bool running() const    = 0;

        virtual std::shared_ptr<Document> document( const std::string& documentId, const caffa::Session* session ) = 0;
        virtual std::shared_ptr<const Document>            document( const std::string&    documentId,
                                                                     const caffa::Session* session ) const         = 0;
        virtual std::list<std::shared_ptr<Document>>       documents( const caffa::Session* session )              = 0;
        virtual std::list<std::shared_ptr<const Document>> documents( const caffa::Session* session ) const        = 0;

        [[nodiscard]] virtual std::list<std::shared_ptr<Document>> defaultDocuments() const = 0;

        [[nodiscard]] bool         requiresValidSession() const;
        void                       setRequiresValidSession( bool requiresValidSession );
        [[nodiscard]] virtual bool isValid( const caffa::Session* session ) const = 0;

        [[nodiscard]] virtual bool       readyForSession( caffa::Session::Type type ) const   = 0;
        virtual std::shared_ptr<Session> createSession( caffa::Session::Type type )           = 0;
        [[nodiscard]] virtual bool       hasActiveSessions() const                            = 0;
        virtual std::shared_ptr<Session> getExistingSession( const std::string& sessionUuid ) = 0;
        [[nodiscard]] virtual std::shared_ptr<const Session> getExistingSession( const std::string& sessionUuid ) const = 0;
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