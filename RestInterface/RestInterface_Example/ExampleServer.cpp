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
#include "cafSession.h"

#include "DemoObject.h"

#include <chrono>
#include <iostream>

#ifndef CAFFA_VERSION_MAJOR
#define CAFFA_VERSION_MAJOR -1
#endif

#ifndef CAFFA_VERSION_MINOR
#define CAFFA_VERSION_MINOR -2
#endif

#ifndef CAFFA_VERSION_PATCH
#define CAFFA_VERSION_PATCH -3
#endif
class ServerApp : public caffa::rpc::RestServerApplication
{
public:
    ServerApp( int port, int threads )
        : caffa::rpc::RestServerApplication( port, threads )
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

    std::shared_ptr<caffa::Document> document( const std::string& documentId, const caffa::Session* session ) override
    {
        CAFFA_TRACE( "Trying to get document with id '" << documentId << "' while our main document is called "
                                                        << m_demoDocument->id() );
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument;
        else
            return nullptr;
    }
    std::shared_ptr<const caffa::Document> document( const std::string& documentId, const caffa::Session* session ) const override
    {
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument;
        else
            return nullptr;
    }
    std::list<std::shared_ptr<caffa::Document>> documents( const caffa::Session* session ) override
    {
        return { document( "", session ) };
    }
    std::list<std::shared_ptr<const caffa::Document>> documents( const caffa::Session* session ) const override
    {
        return { document( "", session ) };
    }

    bool hasActiveSessions() const override { return m_session && !m_session->isExpired(); }

    bool readyForSession( caffa::Session::Type type ) const override
    {
        if ( type == caffa::Session::Type::INVALID ) return false;

        if ( type == caffa::Session::Type::REGULAR )
        {
            if ( m_session && !m_session->isExpired() )
            {
                return false;
            }
            return true;
        }
        return true;
    }
    caffa::SessionMaintainer createSession( caffa::Session::Type type ) override
    {
        if ( m_session )
        {
            if ( !m_session->isExpired() )
            {
                throw std::runtime_error( "We already have a session and only allow one at a time!" );
            }
            else
            {
                CAFFA_WARNING( "Had session " << m_session->uuid() << " but it has not been kept alive, so destroying it" );
            }
        }
        m_session = caffa::Session::create( type, std::chrono::seconds( 60 ) );
        return caffa::SessionMaintainer( m_session );
    }

    caffa::SessionMaintainer getExistingSession( const std::string& sessionUuid ) override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return caffa::SessionMaintainer( m_session );
        }
        return caffa::SessionMaintainer( nullptr );
    }

    caffa::ConstSessionMaintainer getExistingSession( const std::string& sessionUuid ) const override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return caffa::ConstSessionMaintainer( m_session );
        }
        return caffa::ConstSessionMaintainer( nullptr );
    }

    void changeSession( caffa::not_null<caffa::Session*> session, caffa::Session::Type newType ) override
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

private:
    void onStartup() override { CAFFA_INFO( "Starting Server" ); }
    void onShutdown() override { CAFFA_INFO( "Shutting down Server" ); }

private:
    std::shared_ptr<DemoDocument> m_demoDocument;

    std::shared_ptr<caffa::Session> m_session;
};

int main( int argc, char* argv[] )
{
    // Check command line arguments.
    if ( argc != 3 )
    {
        std::cerr << "Usage: ExampleServer <port> <threads>\n"
                  << "Example:\n"
                  << "    ExampleServer 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const port    = static_cast<unsigned short>( std::atoi( argv[1] ) );
    auto const threads = std::max<int>( 1, std::atoi( argv[2] ) );

    caffa::Logger::setApplicationLogLevel( caffa::Logger::Level::debug );

    // Create and launch a listening port
    auto serverApp = std::make_shared<ServerApp>( port, threads );

    auto session = serverApp->createSession( caffa::Session::Type::REGULAR );
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