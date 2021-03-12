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

#include "../cafGrpcClient.h"
#include "../cafGrpcClientObjectFactory.h"
#include "../cafGrpcObjectClientCapability.h"

#include "DemoObject.h"

#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    std::string hostname = argc == 2 ? argv[1] : "localhost";
    int  portNumber = 50000;
    
    std::cout << "Launching Client connecting to " << hostname << ":" << portNumber << std::endl;
    auto client = std::make_unique<caf::rpc::Client>( hostname, portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get());
    std::cout << "Getting document" << std::endl;
    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );

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
