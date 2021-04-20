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

#include "cafGrpcClient.h"
#include "cafGrpcClientApplication.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcObjectClientCapability.h"

#include "cafLogger.h"

#include "DemoObject.h"

#include <cstdlib>
#include <iostream>

class ClientApp : public caffa::rpc::ClientApplication
{
public:
    ClientApp( const std::string& hostname, int port )
        : caffa::rpc::ClientApplication( hostname, port )
    {
    }
    ~ClientApp() = default;

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    std::string name() const override { return "Client Test Example"; }

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
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    std::string hostname   = argc >= 2 ? argv[1] : "localhost";
    int         portNumber = argc >= 3 ? std::atoi( argv[2] ) : 55555;

    caffa::Logger::setApplicationLogLevel( caffa::Logger::Level::INFO );

    auto clientApp = std::make_unique<ClientApp>( hostname, portNumber );
    CAFFA_INFO( "Launching Client connecting to " << hostname << ":" << portNumber );

    auto client         = clientApp->client();
    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    if ( !clientDocument )
    {
        CAFFA_ERROR( "Failed to get main document" );
        return 1;
    }
    bool worked = client->ping();
    CAFFA_ASSERT( worked );
    auto   clientVector   = clientDocument->demoObject()->floatVector();
    size_t numberOfFloats = clientVector.size();
    size_t MB             = numberOfFloats * sizeof( float ) / ( 1024u * 1024u );
    std::cout << "Transferred " << numberOfFloats << " floats for a total of " << MB << " MB" << std::endl;

    clientDocument->demoObject()->setFloatVector( { 123.0, 45.1, 8.32 } );
    return 0;
}
