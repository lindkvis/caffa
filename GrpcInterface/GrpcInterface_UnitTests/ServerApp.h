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

#include "DemoObject.h"

#include "cafGrpcServer.h"
#include "cafGrpcServerApplication.h"
#include "cafLogger.h"
#include "cafSession.h"

#include <chrono>
#include <stdexcept>

class ServerApp : public caffa::rpc::ServerApplication
{
public:
    static int         s_port;
    static std::string s_serverCertFile;
    static std::string s_serverKeyFile;
    static std::string s_caCertFile;
    static std::string s_clientCertFile;
    static std::string s_clientKeyFile;

    ServerApp( int                port,
               const std::string& serverCertFile = "",
               const std::string& serverKeyFile  = "",
               const std::string& caCertFile     = "" )
        : caffa::rpc::ServerApplication( port, serverCertFile, serverKeyFile, caCertFile )
        , m_demoDocument( std::make_unique<DemoDocument>() )
        , m_demoDocumentWithNonScriptableMember( std::make_unique<DemoDocumentWithNonScriptableMember>() )
    {
    }
    static std::shared_ptr<ServerApp> instance();

    std::string name() const override { return "ServerTest"; }
    int         majorVersion() const override { return 1; }
    int         minorVersion() const override { return 0; }
    int         patchVersion() const override { return 0; }

    caffa::Document* document( const std::string& documentId, const caffa::Session* session ) override
    {
        CAFFA_TRACE( "Looking for " << documentId );
        for ( auto document : documents( session ) )
        {
            CAFFA_TRACE( "Found document: " << document->id() );
            if ( documentId.empty() || documentId == document->id() )
            {
                CAFFA_TRACE( "Match!!" );
                return document;
            }
        }
        return nullptr;
    }
    const caffa::Document* document( const std::string& documentId, const caffa::Session* session ) const override
    {
        CAFFA_TRACE( "Looking for " << documentId );
        for ( auto document : documents( session ) )
        {
            CAFFA_TRACE( "Found document: " << document->id() );
            if ( documentId.empty() || documentId == document->id() )
            {
                CAFFA_TRACE( "Match!!" );
                return document;
            }
        }
        return nullptr;
    }
    std::list<caffa::Document*> documents( const caffa::Session* session ) override
    {
        return { m_demoDocument.get(), m_demoDocumentWithNonScriptableMember.get() };
    }
    std::list<const caffa::Document*> documents( const caffa::Session* session ) const override
    {
        return { m_demoDocument.get(), m_demoDocumentWithNonScriptableMember.get() };
    }

    caffa::SessionMaintainer createSession( caffa::Session::Type type ) override
    {
        if ( type == caffa::Session::Type::REGULAR )
        {
            if ( m_session )
            {
                if ( !m_session->isExpired() )
                {
                    throw std::runtime_error( "We already have a session and only allow one at a time!" );
                }
                else
                {
                    CAFFA_DEBUG( "Had session " << m_session->uuid()
                                                << " but it has not been kept alive, so destroying it" );
                }
            }
            m_session = caffa::Session::create( type );
            return caffa::SessionMaintainer( m_session );
        }
        else
        {
            auto observingSession = caffa::Session::create( type, std::chrono::seconds( 1 ) );
            m_observeringSessions.push_back( observingSession );
            return caffa::SessionMaintainer( observingSession );
        }
    }

    caffa::SessionMaintainer getExistingSession( const std::string& sessionUuid ) override
    {
        if ( m_session && m_session->uuid() == sessionUuid && !m_session->isExpired() )
        {
            return caffa::SessionMaintainer( m_session );
        }

        for ( auto& observingSession : m_observeringSessions )
        {
            if ( observingSession && observingSession->uuid() == sessionUuid && !observingSession->isExpired() )
            {
                return caffa::SessionMaintainer( observingSession );
            }
        }

        return caffa::SessionMaintainer();
    }

    caffa::ConstSessionMaintainer getExistingSession( const std::string& sessionUuid ) const override
    {
        if ( m_session && m_session->uuid() == sessionUuid && !m_session->isExpired() )
        {
            return caffa::ConstSessionMaintainer( m_session );
        }

        for ( auto& observingSession : m_observeringSessions )
        {
            if ( observingSession && observingSession->uuid() == sessionUuid && !observingSession->isExpired() )
            {
                return caffa::ConstSessionMaintainer( observingSession );
            }
        }

        return caffa::ConstSessionMaintainer();
    }

    void destroySession( const std::string& sessionUuid )
    {
        CAFFA_TRACE( "Attempting to destroy session " << sessionUuid );
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            m_session.reset();
            CAFFA_TRACE( "Session " << sessionUuid << " destroyed!" );
        }
        else
        {
            auto it = std::find_if( m_observeringSessions.begin(),
                                    m_observeringSessions.end(),
                                    [sessionUuid]( auto&& observingSession )
                                    { return observingSession && observingSession->uuid() == sessionUuid; } );

            if ( it != m_observeringSessions.end() )
            {
                m_observeringSessions.erase( it );
                return;
            }

            throw std::runtime_error( std::string( "Failed to destroy session '" ) + sessionUuid + "'" );
        }
    }

private:
    void onStartup() override { CAFFA_DEBUG( "Starting Server" ); }
    void onShutdown() override { CAFFA_DEBUG( "Shutting down Server" ); }

private:
    std::unique_ptr<DemoDocument>                        m_demoDocument;
    std::unique_ptr<DemoDocumentWithNonScriptableMember> m_demoDocumentWithNonScriptableMember;

    std::shared_ptr<caffa::Session>            m_session;
    std::list<std::shared_ptr<caffa::Session>> m_observeringSessions;
};
