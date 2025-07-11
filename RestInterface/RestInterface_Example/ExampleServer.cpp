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

#include "cafLogger.h"
#include "cafRestServer.h"
#include "cafRestServerApplication.h"
#include "cafRestSession.h"
#include "cafSession.h"

#include "DemoObject.h"

#include <chrono>
#include <iostream>
#include <set>

#ifndef CAFFA_VERSION_MAJOR
#define CAFFA_VERSION_MAJOR -1
#endif

#ifndef CAFFA_VERSION_MINOR
#define CAFFA_VERSION_MINOR -2
#endif

#ifndef CAFFA_VERSION_PATCH
#define CAFFA_VERSION_PATCH -3
#endif

using namespace caffa;
using namespace std::chrono_literals;

/**
 * A very simple, *not at all* production ready authenticator.
 * It uses plain text, unhashed passwords.
 * DO NOT USE THIS FOR PRODUCTION CODE.
 */
class DemoAuthenticator : public rpc::RestAuthenticator
{
public:
    DemoAuthenticator( const std::set<std::string>& validAuthentications,
                       const std::string&           cert,
                       const std::string&           key,
                       const std::string&           dh )
        : m_validAuthentications( validAuthentications )
        , m_cert( cert )
        , m_key( key )
        , m_dh( dh )
    {
    }

    std::string sslCertificate() const override { return m_cert; }
    std::string sslKey() const override { return m_key; }
    std::string sslDhParameters() const override { return m_dh; }

    bool authenticate( const std::string& authenticationHeader ) const override
    {
        CAFFA_DEBUG( "Got authorisation header: " << authenticationHeader );
        return m_validAuthentications.contains( authenticationHeader );
    }

private:
    std::set<std::string> m_validAuthentications;

    std::string m_cert;
    std::string m_key;
    std::string m_dh;
};

class ServerApp : public rpc::RestServerApplication
{
public:
    ServerApp( int port, int threads, std::shared_ptr<const rpc::RestAuthenticator> authenticator )
        : rpc::RestServerApplication( "0.0.0.0", port, threads, authenticator )
        , m_demoDocument( std::make_shared<DemoDocument>() )
    {
    }
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    std::string name() const override { return "ServerTest"; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int majorVersion() const override { return CAFFA_VERSION_MAJOR; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int minorVersion() const override { return CAFFA_VERSION_MINOR; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int patchVersion() const override { return CAFFA_VERSION_PATCH; }

    std::string description() const override { return "Example Server for Caffa"; }
    std::string contactEmail() const override { return "example@thisdomaindoesnotexist.com"; }

    std::shared_ptr<Document> document( const std::string& documentId, const Session* session ) override
    {
        CAFFA_TRACE( "Trying to get document with id '" << documentId << "' while our main document is called "
                                                        << m_demoDocument->id() );
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument;
        else
            return nullptr;
    }
    std::shared_ptr<const Document> document( const std::string& documentId, const Session* session ) const override
    {
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument;
        else
            return nullptr;
    }
    std::list<std::shared_ptr<Document>> documents( const Session* session ) override
    {
        return { document( "", session ) };
    }
    std::list<std::shared_ptr<const Document>> documents( const Session* session ) const override
    {
        return { document( "", session ) };
    }

    std::list<std::shared_ptr<caffa::Document>> defaultDocuments() const override
    {
        return { std::make_shared<DemoDocument>() };
    }

    bool hasActiveSessions() const override { return m_session && isValid( m_session.get() ); }

    bool readyForSession( Session::Type type ) const override
    {
        if ( type == Session::Type::INVALID ) return false;

        if ( type == Session::Type::REGULAR )
        {
            if ( m_session && isValid( m_session.get() ) )
            {
                return false;
            }
            return true;
        }
        return true;
    }

    std::shared_ptr<caffa::Session> createSession( Session::Type type ) override
    {
        if ( m_session )
        {
            if ( isValid( m_session.get() ) )
            {
                throw std::runtime_error( "We already have a session and only allow one at a time!" );
            }
            else
            {
                CAFFA_WARNING( "Had session " << m_session->uuid() << " but it has not been kept alive, so destroying it" );
            }
        }
        m_session = Session::create( type );
        return m_session;
    }

    std::shared_ptr<caffa::Session> getExistingSession( const std::string& sessionUuid ) override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return m_session;
        }
        return nullptr;
    }

    std::shared_ptr<const caffa::Session> getExistingSession( const std::string& sessionUuid ) const override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return m_session;
        }
        return nullptr;
    }

    void changeSession( not_null<Session*> session, Session::Type newType ) override
    {
        throw std::runtime_error( "Cannot change sessions in example app" );
    }

    void destroySession( const std::string& sessionUuid ) override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            m_session.reset();
        }
        else
        {
            throw std::runtime_error( "Failed to destroy session " + sessionUuid );
        }
    }

    bool isValid( const caffa::Session* session ) const override
    {
        return ( std::chrono::steady_clock::now() - session->lastKeepAlive() ) < 60s;
    }

private:
    void onStartup() override { CAFFA_INFO( "Starting Server" ); }
    void onShutdown() override { CAFFA_INFO( "Shutting down Server" ); }

private:
    std::shared_ptr<DemoDocument> m_demoDocument;

    std::shared_ptr<Session> m_session;
};

int main( int argc, char* argv[] )
{
    // Check command line arguments.
    if ( argc < 3 )
    {
        std::cerr << "Usage: ExampleServer <port> <threads> <cert> <key> <dh>\n"
                  << "Example:\n"
                  << "    ExampleServer 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const port    = static_cast<unsigned short>( std::atoi( argv[1] ) );
    auto const threads = std::max<int>( 1, std::atoi( argv[2] ) );

    std::string cert, key, dh;
    if ( argc > 3 )
    {
        if ( argc != 6 )
        {
            std::cerr << "If using SSL you need to provide three extra arguments" << std::endl;
            return EXIT_FAILURE;
        }
        cert = caffa::rpc::RpcApplication::readKeyOrCertificate( argv[3] );
        key  = caffa::rpc::RpcApplication::readKeyOrCertificate( argv[4] );
        dh   = caffa::rpc::RpcApplication::readKeyOrCertificate( argv[5] );
    }

    Logger::setApplicationLogLevel( Logger::Level::debug );

    // Create and launch a listening port
    auto serverApp =
        std::make_shared<ServerApp>( port,
                                     threads,
                                     std::make_shared<DemoAuthenticator>( std::set<std::string>{ "test:password" },
                                                                          cert,
                                                                          key,
                                                                          dh ) );

    auto session = serverApp->createSession( Session::Type::REGULAR );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    CAFFA_ASSERT( serverDocument != nullptr );

    serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
    serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
    serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
    std::vector<float> serverVector;
    size_t             numberOfFloats = 32u;
    serverVector.reserve( numberOfFloats );
    for ( size_t i = 0; i < numberOfFloats; ++i )
    {
        serverVector.push_back( (float)i );
    }

    auto demoObject = serverDocument->demoObject();

    demoObject->intVector.setValue( { 42, 13, 333 } );
    demoObject->floatVector.setValue( serverVector );
    demoObject->boolVector.setValue( { true, false, false, true } );

    serverApp->destroySession( session->uuid() );

    serverApp->setRequiresValidSession( false );
    serverApp->run();

    return EXIT_SUCCESS;
}