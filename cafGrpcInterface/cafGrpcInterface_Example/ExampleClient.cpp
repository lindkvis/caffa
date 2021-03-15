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

#include "../cafGrpcClientApplication.h"
#include "../cafGrpcClient.h"
#include "../cafGrpcClientObjectFactory.h"
#include "../cafGrpcObjectClientCapability.h"

#include "DemoObject.h"

#include <cstdlib>
#include <iostream>

class ClientApp : public caf::rpc::ClientApplication
{
public:
    ClientApp( const std::string& hostname, int port )
        : caf::rpc::ClientApplication( hostname, port )
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
    std::string hostname = argc >= 2 ? argv[1] : "localhost";
    int         portNumber = argc >= 3 ? std::atoi( argv[2] ) : 55555;

    auto clientApp = std::make_unique<ClientApp>(hostname, portNumber);    
    std::cout << "Launching Client connecting to " << hostname << ":" << portNumber << std::endl;

    if (argc >= 4)
    {
        int packageByteSize = std::atoi(argv[3]);
        clientApp->setPackageByteSize((size_t) packageByteSize);
        std::cout << "Package size: " << packageByteSize << std::endl;
    }

    auto client = clientApp->client();
    std::cout << "Getting document" << std::endl;
    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    if (!clientDocument)
    {
        std::cerr << "Failed to get main document" << std::endl;
        return 1;
    }
    {
        auto start_time = std::chrono::system_clock::now();
        auto clientVector    = clientDocument->m_demoObject->getIntVector();
        auto end_time   = std::chrono::system_clock::now();
        auto duration   = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
        std::cout << "Client vector: " << clientVector[0] << std::endl;
        assert( clientVector.size() == 1u && clientVector[0] == 42 );

        std::cout << "Getting vector containing single integer took " << duration << "µs" << std::endl;
    }

    {
        auto start_time = std::chrono::system_clock::now();
        auto clientVector    = clientDocument->m_demoObject->getFloatVector();
        auto end_time   = std::chrono::system_clock::now();
        auto duration   = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        size_t numberOfFloats = clientVector.size();
        size_t MB = numberOfFloats * sizeof(float) / (1024u * 1024u);
        std::cout << "Transferred " << numberOfFloats << " floats for a total of " << MB << " MB" << std::endl;        
        std::cout << "Time spent: " << duration << "ms" << std::endl;
        double fps = static_cast<float>( numberOfFloats ) / static_cast<float>( duration ) * 1000;
        std::cout << "floats per second: " << fps << std::endl;
        std::cout << "MB per second: " << static_cast<float>(MB) / static_cast<float>(duration) * 1000 << std::endl;
    }
    return 0;
}
