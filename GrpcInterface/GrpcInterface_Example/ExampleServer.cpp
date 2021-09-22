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
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    std::string name() const override { return "ServerTest Example"; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int majorVersion() const override { return 5; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int minorVersion() const override { return 1; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    int patchVersion() const override { return 0; }

    caffa::Document*            document( const std::string& documentId ) override { return &m_demoDocument; }
    const caffa::Document*      document( const std::string& documentId ) const override { return &m_demoDocument; }
    std::list<caffa::Document*> documents() override { return { document( "" ) }; }
    std::list<const caffa::Document*> documents() const override { return { document( "" ) }; }

private:
    void onStartup() override { CAFFA_INFO( "Starting Server" ); }
    void onShutdown() override { CAFFA_INFO( "Shutting down Server" ); }

private:
    DemoDocument m_demoDocument;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    int  portNumber = argc >= 2 ? std::atoi( argv[1] ) : 55555;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    if ( argc >= 3 )
    {
        int packageByteSize = std::atoi( argv[2] );
        serverApp->setPackageByteSize( (size_t)packageByteSize );
        CAFFA_DEBUG( "Using package size " << packageByteSize << "B" );
    }

    CAFFA_INFO( "Launching Server listening on port " << portNumber );

    caffa::Logger::setApplicationLogLevel( caffa::Logger::Level::TRACE );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );

    std::vector<float> serverVector;
    size_t             numberOfFloats = 1024u * 1024u * 4;
    serverVector.reserve( numberOfFloats );
    for ( size_t i = 0; i < numberOfFloats; ++i )
    {
        serverVector.push_back( (float)i );
    }

    serverDocument->demoObject()->setIntVector( { 42 } );
    serverDocument->demoObject()->setFloatVector( serverVector );
    CAFFA_DEBUG( "Running server thread" );
    serverApp->run();
}
