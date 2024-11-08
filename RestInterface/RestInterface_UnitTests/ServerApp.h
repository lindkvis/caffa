// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- 3D-Radar AS
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

#include "DemoObject.h"

#include "cafLogger.h"
#include "cafRestServer.h"
#include "cafRestServerApplication.h"
#include "cafRestSession.h"
#include "cafSession.h"

#include <chrono>
#include <mutex>
#include <stdexcept>

using namespace std::chrono_literals;

class UnitTestAuthenticator : public caffa::rpc::RestAuthenticator
{
public:
    std::string sslCertificate() const override { return ""; }
    std::string sslKey() const override { return ""; }
    std::string sslDhParameters() const override { return ""; }
    bool        authenticate( const std::string& authenticationHeader ) const override { return true; }
};

class ServerApp : public caffa::rpc::RestServerApplication
{
public:
    static int s_port;

    ServerApp( int port, int threads )
        : caffa::rpc::RestServerApplication( "0.0.0.0", port, threads, std::make_shared<UnitTestAuthenticator>() )
        , m_demoDocument( std::make_unique<DemoDocument>() )
        , m_demoDocumentWithNonScriptableMember( std::make_unique<DemoDocumentWithNonScriptableMember>() )
    {
    }
    static std::shared_ptr<ServerApp> instance();

    std::string name() const override { return "ServerTest"; }
    int         majorVersion() const override { return 1; }
    int         minorVersion() const override { return 0; }
    int         patchVersion() const override { return 0; }
    std::string description() const override { return "Unit Test Server for Caffa"; }
    std::string contactEmail() const override { return "test@thisdomaindoesnotexist.com"; }

    std::shared_ptr<caffa::Document> document( const std::string& documentId, const caffa::Session* session ) override
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
    std::shared_ptr<const caffa::Document> document( const std::string& documentId, const caffa::Session* session ) const override
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
    std::list<std::shared_ptr<caffa::Document>> documents( const caffa::Session* session ) override
    {
        return { m_demoDocument, m_demoDocumentWithNonScriptableMember };
    }
    std::list<std::shared_ptr<const caffa::Document>> documents( const caffa::Session* session ) const override
    {
        return { m_demoDocument, m_demoDocumentWithNonScriptableMember };
    }

    std::list<std::shared_ptr<caffa::Document>> defaultDocuments() const override
    {
        return { std::make_shared<DemoDocument>(), std::make_shared<DemoDocumentWithNonScriptableMember>() };
    }

    bool hasActiveSessions() const override
    {
        std::scoped_lock lock( m_sessionMutex );

        return m_session || !m_observingSessions.empty();
    }

    bool readyForSession( caffa::Session::Type type ) const override
    {
        if ( type == caffa::Session::Type::INVALID ) return false;

        std::scoped_lock lock( m_sessionMutex );

        if ( type == caffa::Session::Type::REGULAR )
        {
            if ( m_session )
            {
                if ( isValidUnlocked( m_session.get() ) )
                {
                    return false;
                }
            }
            return true;
        }
        return true;
    }

    bool isValid( const caffa::Session* session ) const override
    {
        std::scoped_lock lock( m_sessionMutex );
        return isValidUnlocked( session );
    }

    bool isValidUnlocked( const caffa::Session* session ) const
    {
        const auto now = std::chrono::steady_clock::now();
        return session && ( ( now - session->lastKeepAlive() ) < 2s );
    }

    std::shared_ptr<caffa::Session> createSession( caffa::Session::Type type = caffa::Session::Type::REGULAR ) override
    {
        if ( type == caffa::Session::Type::INVALID )
            throw std::runtime_error( "Cannot create sessions of type 'INVALID'" );

        std::scoped_lock lock( m_sessionMutex );

        if ( type == caffa::Session::Type::REGULAR )
        {
            if ( m_session )
            {
                if ( isValidUnlocked( m_session.get() ) )
                {
                    throw std::runtime_error( "We already have a regular session and only allow one at a time!" );
                }
                else
                {
                    CAFFA_DEBUG( "Had session " << m_session->uuid()
                                                << " but it has not been kept alive, so destroying it" );
                }
            }
            m_session = caffa::Session::create( type );
            return m_session;
        }
        auto observingSession = caffa::Session::create( type );
        m_observingSessions.push_back( observingSession );
        return observingSession;
    }

    std::shared_ptr<caffa::Session> getExistingSession( const std::string& sessionUuid ) override
    {
        std::scoped_lock lock( m_sessionMutex );
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return m_session;
        }
        for ( auto observingSession : m_observingSessions )
        {
            if ( observingSession && observingSession->uuid() == sessionUuid && isValidUnlocked( observingSession.get() ) )
            {
                return observingSession;
            }
        }
        return nullptr;
    }

    std::shared_ptr<const Session> getExistingSession( const std::string& sessionUuid ) const override
    {
        std::scoped_lock lock( m_sessionMutex );

        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return m_session;
        }

        for ( auto observingSession : m_observingSessions )
        {
            if ( observingSession && observingSession->uuid() == sessionUuid && isValidUnlocked( observingSession.get() ) )
            {
                return observingSession;
            }
        }

        return nullptr;
    }

    void changeSession( caffa::not_null<caffa::Session*> session, caffa::Session::Type newType ) override
    {
        throw std::runtime_error( "Cannot change sessions in test app" );
    }

    void destroySession( const std::string& sessionUuid ) override
    {
        std::scoped_lock lock( m_sessionMutex );

        CAFFA_TRACE( "Attempting to destroy session " << sessionUuid );
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            m_session.reset();
            CAFFA_TRACE( "Session " << sessionUuid << " destroyed!" );
        }
        else
        {
            auto it = std::find_if( m_observingSessions.begin(),
                                    m_observingSessions.end(),
                                    [sessionUuid]( auto&& observingSession )
                                    { return observingSession && observingSession->uuid() == sessionUuid; } );

            if ( it != m_observingSessions.end() )
            {
                m_observingSessions.erase( it );
                return;
            }

            throw std::runtime_error( std::string( "Failed to destroy session '" ) + sessionUuid + "'" );
        }
    }

private:
    void onStartup() override { CAFFA_DEBUG( "Server Starting Up...." ); }
    void onShutdown() override { CAFFA_DEBUG( "Server Shutting Down..." ); }

private:
    std::shared_ptr<DemoDocument>                        m_demoDocument;
    std::shared_ptr<DemoDocumentWithNonScriptableMember> m_demoDocumentWithNonScriptableMember;

    std::shared_ptr<caffa::Session>            m_session;
    std::list<std::shared_ptr<caffa::Session>> m_observingSessions;
    mutable std::mutex                         m_sessionMutex;
};
