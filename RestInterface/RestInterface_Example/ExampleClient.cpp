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

#include "cafRestClient.h"
#include "cafRestClientApplication.h"
#include "cafRpcClientPassByRefObjectFactory.h"

#include "cafLogger.h"

#include "DemoObject.h"

#include <cstdlib>
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

class ClientApp : public caffa::rpc::RestClientApplication
{
public:
    ClientApp( const std::string& hostname, int port )
        : caffa::rpc::RestClientApplication( hostname, port )
    {
    }
    ~ClientApp() override = default;

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    std::string name() const override { return "Client Test Example"; }

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

    std::string description() const override { return "Example Client for Caffa"; }
    std::string contactEmail() const override { return "client@thisdomaindoesnotexist.com"; }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    std::string hostname   = argc >= 2 ? argv[1] : "localhost";
    int         portNumber = argc >= 3 ? std::atoi( argv[2] ) : 50000;

    caffa::Logger::setApplicationLogLevel( caffa::Logger::Level::info );

    auto factory = caffa::rpc::ClientPassByRefObjectFactory::instance();
    factory->registerBasicAccessorCreators<caffa::AppEnum<DemoObject::TestEnumType>>();

    auto clientApp = std::make_unique<ClientApp>( hostname, portNumber );
    CAFFA_INFO( "Launching Client connecting to " << hostname << ":" << portNumber );

    auto client  = clientApp->client();
    auto appInfo = clientApp->appInfo();

    CAFFA_INFO( "Found server " << appInfo.name << " version " << appInfo.version_string() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    if ( !clientDocument )
    {
        CAFFA_ERROR( "Failed to get main document" );
        return 1;
    }

    auto demoObject      = clientDocument->demoObject();
    demoObject->intField = 1337;

    CAFFA_INFO( "Int field set to:  " << *demoObject->intField );

    return 0;
}
