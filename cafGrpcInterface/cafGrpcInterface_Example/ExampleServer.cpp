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

#include "../cafGrpcServer.h"
#include "../cafGrpcServerApplication.h"

#include "DemoObject.h"

#include <filesystem>
#include <iostream>
#include <random>
#include <string>

class ServerApp : public caf::GrpcServerApplication
{
public:
    ServerApp( int port )
        : caf::GrpcServerApplication( port )
    {
    }
    ~ServerApp() = default;

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

    caf::PdmDocument*            document( const std::string& documentId ) override { return &m_demoDocument; }
    const caf::PdmDocument*      document( const std::string& documentId ) const override { return &m_demoDocument; }
    std::list<caf::PdmDocument*> documents() override { return { document( "" ) }; }
    std::list<const caf::PdmDocument*> documents() const override { return { document( "" ) }; }

private:
    DemoDocument m_demoDocument;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    int  portNumber = 55555;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    std::cout << "Launching Server listening on port " << portNumber << std::endl;


    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
 
    std::vector<float> serverVector;
    std::mt19937       rng;
    size_t             numberOfFloats = 1024u * 1024u * 128;
    serverVector.reserve( numberOfFloats );
    for (size_t i = 0; i < numberOfFloats; ++i)
    {
        serverVector.push_back( (float) rng() );
    }

    serverDocument->m_demoObject->setIntVector({42});
    serverDocument->m_demoObject->setFloatVector( serverVector );
    std::cout << "Running server thread" << std::endl;
    serverApp->run();
}
