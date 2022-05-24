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
    int majorVersion() const override { return 1; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int minorVersion() const override { return 0; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int patchVersion() const override { return 0; }

    caffa::Document* document( const std::string& documentId, const std::string& sessionUuid = "" ) override
    {
        CAFFA_TRACE( "Trying to get document with id " << documentId << " while our main document is called "
                                                       << m_demoDocument->id() );
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument.get();
        else
            return nullptr;
    }
    const caffa::Document* document( const std::string& documentId, const std::string& sessionUuid = "" ) const override
    {
        if ( documentId.empty() || documentId == m_demoDocument->id() )
            return m_demoDocument.get();
        else
            return nullptr;
    }
    std::list<caffa::Document*> documents( const std::string& sessionUuid = "" ) override
    {
        return { document( "", sessionUuid ) };
    }
    std::list<const caffa::Document*> documents( const std::string& sessionUuid = "" ) const override
    {
        return { document( "", sessionUuid ) };
    }

    void resetToDefaultData() override { m_demoDocument = std::make_unique<DemoDocument>(); }

    caffa::Session* createSession() override
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
        m_session = std::make_unique<caffa::Session>();
        return m_session.get();
    }

    caffa::Session* getExistingSession( const std::string& sessionUuid ) override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            return m_session.get();
        }
        return nullptr;
    }

    void keepAliveSession( const std::string& sessionUuid ) override
    {
        if ( m_session && m_session->uuid() == sessionUuid )
        {
            m_session->updateKeepAlive();
        }
        else
        {
            CAFFA_ERROR( "Session does not exist " << sessionUuid );
            throw std::runtime_error( std::string( "Session does not exist'" ) + sessionUuid + "'" );
        }
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

    std::unique_ptr<caffa::Session> m_session;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    caffa::Logger::setApplicationLogLevel( caffa::Logger::Level::TRACE );

    int  portNumber = argc >= 2 ? std::atoi( argv[1] ) : 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    if ( argc >= 3 )
    {
        int packageByteSize = std::atoi( argv[2] );
        serverApp->setPackageByteSize( (size_t)packageByteSize );
        CAFFA_DEBUG( "Using package size " << packageByteSize << "B" );
    }

    CAFFA_INFO( "Launching Server listening on port " << portNumber );

    DemoDocument* serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
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
    CAFFA_DEBUG( "Running server thread" );
    serverApp->run();
}
