//##################################################################################################
//
//   Caffa
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
//##################################################################################################

#include "cafGrpcServer.h"
#include "cafGrpcServerApplication.h"
#include "cafSession.h"

#include "cafLogger.h"

#include "DemoObject.h"

#include <cstdlib>
#include <iostream>
#include <string>

#ifndef CAFFA_VERSION_MAJOR
#define CAFFA_VERSION_MAJOR -1
#endif

#ifndef CAFFA_VERSION_MINOR
#define CAFFA_VERSION_MINOR -2
#endif

#ifndef CAFFA_VERSION_PATCH
#define CAFFA_VERSION_PATCH -3
#endif
class ServerApp : public caffa::rpc::ServerApplication
{
public:
    ServerApp( int port )
        : caffa::rpc::ServerApplication( port )
        , m_demoDocument( std::make_unique<DemoDocument>() )
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

    caffa::Document* document( const std::string& documentId, const caffa::Session* session ) override
    {
        CAFFA_TRACE( "Trying to get document with id '" << documentId << "' while our main document is called "
                                                        << m_demoDocument->id() );
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument.get();
        else
            return nullptr;
    }
    const caffa::Document* document( const std::string& documentId, const caffa::Session* session ) const override
    {
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument.get();
        else
            return nullptr;
    }
    std::list<caffa::Document*> documents( const caffa::Session* session ) override
    {
        return { document( "", session ) };
    }
    std::list<const caffa::Document*> documents( const caffa::Session* session ) const override
    {
        return { document( "", session ) };
    }

    void resetToDefaultData() override { m_demoDocument = std::make_unique<DemoDocument>(); }

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
                CAFFA_DEBUG( "Had session " << m_session->uuid() << " but it has not been kept alive, so destroying it" );
            }
        }
        m_session = caffa::Session::create( type );
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

    void destroySession( const std::string& sessionUuid )
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            m_session.reset();
        }
    }

private:
    void onStartup() override { CAFFA_INFO( "Starting Server" ); }
    void onShutdown() override { CAFFA_INFO( "Shutting down Server" ); }

private:
    std::unique_ptr<DemoDocument> m_demoDocument;

    std::shared_ptr<caffa::Session> m_session;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    caffa::Logger::setApplicationLogLevel( caffa::Logger::Level::info );

    int  portNumber = argc >= 2 ? std::atoi( argv[1] ) : 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    if ( argc >= 3 )
    {
        int packageByteSize = std::atoi( argv[2] );
        serverApp->setPackageByteSize( (size_t)packageByteSize );
        CAFFA_DEBUG( "Using package size " << packageByteSize << "B" );
    }

    CAFFA_INFO( "Launching Server v" << serverApp->majorVersion() << "." << serverApp->minorVersion() << "."
                                     << serverApp->patchVersion() << " listening on port " << portNumber );

    auto          session        = serverApp->createSession( caffa::Session::Type::REGULAR );
    DemoDocument* serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    CAFFA_ASSERT( serverDocument != nullptr );
    std::vector<float> serverVector;
    size_t             numberOfFloats = 1024u * 1024u * 4;
    serverVector.reserve( numberOfFloats );
    for ( size_t i = 0; i < numberOfFloats; ++i )
    {
        serverVector.push_back( (float)i );
    }

    DemoObject* demoObject = serverDocument->demoObject();

    demoObject->intVector.setValue( { 42 } );
    demoObject->floatVector.setValue( serverVector );
    demoObject->boolVector.setValue( { true, false, false, true } );

    serverApp->destroySession( session->uuid() );

    CAFFA_DEBUG( "Running server thread" );
    serverApp->run();
}
