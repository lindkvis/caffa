
#include "gtest.h"

#include "Child.h"
#include "DemoObject.h"
#include "Parent.h"
#include "ServerApp.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcClient.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafTypedField.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

class GrpcTest : public ::testing::Test
{
protected:
    GrpcTest()
    {
        caffa::rpc::GrpcClientObjectFactory* factory = caffa::rpc::GrpcClientObjectFactory::instance();
        factory->registerBasicAccessorCreators<caffa::AppEnum<DemoObject::TestEnumType>>();
        serverApp = std::make_unique<ServerApp>( ServerApp::s_port,
                                                 ServerApp::s_serverCertFile,
                                                 ServerApp::s_serverKeyFile,
                                                 ServerApp::s_caCertFile );
    }

    std::unique_ptr<ServerApp> serverApp;
};

TEST_F( GrpcTest, Launch )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    {
        auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                            "localhost",
                                                            ServerApp::s_port,
                                                            ServerApp::s_clientCertFile,
                                                            ServerApp::s_clientKeyFile,
                                                            ServerApp::s_caCertFile );

        caffa::AppInfo appInfo = client->appInfo();
        ASSERT_EQ( serverApp->name(), appInfo.name );

        CAFFA_DEBUG( "Confirmed test results!" );
        bool ok = client->stopServer();
        ASSERT_TRUE( ok );
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    CAFFA_DEBUG( "Waiting for server thread to join" );
    thread.join();
    CAFFA_DEBUG( "Finishing test" );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, Document )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    CAFFA_DEBUG( "Launching Server" );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    CAFFA_DEBUG( "Launching Client" );
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    CAFFA_INFO( "Expect failure to get document with the wrong document ID. Next line should be an error." );
    auto nonExistentDocument = dynamic_cast<DemoDocument*>( serverApp->document( "wrongName", session.get() ) );
    ASSERT_TRUE( nonExistentDocument == nullptr );
    auto blankNameDocument = dynamic_cast<DemoDocument*>( serverApp->document( "", session.get() ) );
    ASSERT_TRUE( blankNameDocument );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    CAFFA_DEBUG( "Now getting client document" );

    try
    {
        std::unique_ptr<caffa::ObjectHandle> objectHandle;
        ASSERT_NO_THROW( objectHandle = client->document( "testDocument" ) );
        auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
        ASSERT_TRUE( clientDocument != nullptr );

        ASSERT_ANY_THROW( auto nonExistentClientDocument = client->document( "wrongName" ) );

        auto clientFileName = clientDocument->fileName();
        CAFFA_DEBUG( "Client Document File Name: " << clientFileName );
        ASSERT_EQ( serverApp->document( "testDocument", session.get() )->fileName(), clientDocument->fileName() );
        ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

        {
            auto serverDescendants = serverDocument->matchingDescendants(
                []( const caffa::ObjectHandle* objectHandle ) -> bool
                { return dynamic_cast<const DemoObject*>( objectHandle ) != nullptr; } );

            auto clientDescendants = clientDocument->matchingDescendants(
                []( const caffa::ObjectHandle* objectHandle ) -> bool
                { return dynamic_cast<const DemoObject*>( objectHandle ) != nullptr; } );

            ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
            for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
                  server_it != serverDescendants.end();
                  ++server_it, ++client_it )
            {
                ASSERT_EQ( ( *server_it )->uuid(), ( *client_it )->uuid() );
            }
        }

        {
            auto serverDescendants = serverDocument->matchingDescendants(
                []( const caffa::ObjectHandle* objectHandle ) -> bool
                { return dynamic_cast<const InheritedDemoObj*>( objectHandle ) != nullptr; } );

            auto clientDescendants = clientDocument->matchingDescendants(
                []( const caffa::ObjectHandle* objectHandle ) -> bool
                { return dynamic_cast<const InheritedDemoObj*>( objectHandle ) != nullptr; } );

            ASSERT_EQ( childCount, serverDescendants.size() );
            ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
            for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
                  server_it != serverDescendants.end();
                  ++server_it, ++client_it )
            {
                ASSERT_EQ( ( *server_it )->uuid(), ( *client_it )->uuid() );
            }
        }
        std::string serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument );
        CAFFA_DEBUG( serverJson );
        std::string clientJson = caffa::JsonSerializer().writeObjectToString( clientDocument );
        CAFFA_DEBUG( clientJson );
        ASSERT_EQ( serverJson, clientJson );

        CAFFA_DEBUG( "Confirmed test results!" );
        bool ok = client->stopServer();
        ASSERT_TRUE( ok );
        CAFFA_DEBUG( "Waiting for server thread to join" );
        thread.join();
        CAFFA_DEBUG( "Finishing test" );
    }
    catch ( const std::runtime_error& e )
    {
        CAFFA_ERROR( "Exception caught: " << e.what() );
        return;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Exception caught" );
        return;
    }
}

TEST_F( GrpcTest, DocumentWithNonScriptableChild )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    CAFFA_DEBUG( "Launching Server" );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    CAFFA_DEBUG( "Launching Client" );
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session = serverApp->getExistingSession( client->sessionUuid() );

    auto serverDocument =
        dynamic_cast<DemoDocumentWithNonScriptableMember*>( serverApp->document( "testDocument2", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    auto objectHandle   = client->document( "testDocument2" );
    auto clientDocument = dynamic_cast<DemoDocumentWithNonScriptableMember*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_EQ( serverDocument->fileName(), clientDocument->fileName() );
    ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    {
        auto serverObject = serverDocument->demoObjectNonScriptable();
        auto clientObject = clientDocument->demoObjectNonScriptable();

        // The objects are not scriptable, so uuid should not match!!
        ASSERT_NE( serverObject->uuid(), clientObject->uuid() );
    }

    {
        auto serverDescendants = serverDocument->matchingDescendants(
            []( const caffa::ObjectHandle* objectHandle ) -> bool
            { return dynamic_cast<const InheritedDemoObj*>( objectHandle ) != nullptr; } );

        auto clientDescendants = clientDocument->matchingDescendants(
            []( const caffa::ObjectHandle* objectHandle ) -> bool
            { return dynamic_cast<const InheritedDemoObj*>( objectHandle ) != nullptr; } );

        ASSERT_EQ( childCount, serverDescendants.size() );
        ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
        for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
              server_it != serverDescendants.end();
              ++server_it, ++client_it )
        {
            ASSERT_EQ( ( *server_it )->uuid(), ( *client_it )->uuid() );
        }
    }

    std::string serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument );
    CAFFA_DEBUG( serverJson );
    std::string clientJson = caffa::JsonSerializer().writeObjectToString( clientDocument );
    CAFFA_DEBUG( clientJson );
    ASSERT_NE( serverJson, clientJson );

    CAFFA_DEBUG( "Confirmed test results!" );
    bool ok = client->stopServer();
    ASSERT_TRUE( ok );
    CAFFA_DEBUG( "Waiting for server thread to join" );
    thread.join();
    CAFFA_DEBUG( "Finishing test" );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, Sync )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<caffa::Document*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    CAFFA_DEBUG( "Client Document File Name: " << clientDocument->fileName() );
    ASSERT_EQ( serverApp->document( "testDocument", session.get() )->fileName(), clientDocument->fileName() );
    ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

    std::string newFileName = "ChangedFileName.txt";
    clientDocument->setFileName( newFileName );
    ASSERT_EQ( newFileName, serverDocument->fileName() );
    ASSERT_EQ( newFileName, clientDocument->fileName() );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, SettingValueWithObserver )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::OBSERVING,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );
    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<caffa::Document*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    CAFFA_DEBUG( "Client Document File Name: " << clientDocument->fileName() );
    ASSERT_EQ( serverApp->document( "testDocument", session.get() )->fileName(), clientDocument->fileName() );
    ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

    std::string newFileName = "ChangedFileName.txt";
    try
    {
        clientDocument->setFileName( newFileName );
        CAFFA_ERROR( "Setting the file name should throw exception" );
    }
    catch ( const std::runtime_error& e )
    {
        CAFFA_INFO( "Setting file name with observing session threw exception as expected" );
    }

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ObjectMethod )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    CAFFA_DEBUG( "Adding object to server" );
    serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    auto serverObjects = serverDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, serverObjects.size() );
    CAFFA_DEBUG( "Getting client objects" );
    auto inheritedObjects = clientDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, inheritedObjects.size() );

    CAFFA_DEBUG( "Listing object methods" );
    auto objectMethods = client->objectMethods( inheritedObjects.front() );
    ASSERT_EQ( (size_t)1, objectMethods.size() );
    std::string methodKeyword = objectMethods.front()->classKeyword();
    CAFFA_TRACE( "Found method: " << methodKeyword );

    std::vector<bool>  boolVector = { true, false, true };
    std::vector<int>   intVector( 10000 );
    std::vector<float> floatVector = { -2.0, 3.0, 4.0, 8.0 };

    std::iota( intVector.begin(), intVector.end(), 0 );

    CAFFA_INFO( "Creating object method" );
    DemoObject_copyObject method( inheritedObjects.front(), 45.3, 43, "AnotherValue", boolVector, intVector, floatVector );
    ASSERT_EQ( method.classKeyword(), methodKeyword );

    CAFFA_INFO( "Execute" );
    auto result = client->execute( &method );
    ASSERT_TRUE( result );
    ASSERT_EQ( true, result->status() );

    CAFFA_DEBUG( "Get double member" );
    ASSERT_EQ( 45.3, serverObjects.front()->doubleField() );
    CAFFA_DEBUG( "Get int member" );
    ASSERT_EQ( 43, serverObjects.front()->intField() );
    ASSERT_EQ( "AnotherValue", serverObjects.front()->stringField() );

    ASSERT_EQ( boolVector, serverObjects.front()->boolVector() );
    CAFFA_INFO( "Comparing integer vector of size " << intVector.size() << " on server" );
    ASSERT_EQ( intVector, serverObjects.front()->intVector() );
    ASSERT_EQ( floatVector, serverObjects.front()->floatVector() );

    auto roundtripIntVector = inheritedObjects.front()->intVector();
    CAFFA_INFO( "Comparing integer vector of size " << intVector.size() << " on client" );
    ASSERT_EQ( intVector, roundtripIntVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ObjectIntGetterAndSetter )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );
    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<int> largeIntVector;
    std::mt19937     rng;
    std::generate_n( std::back_inserter( largeIntVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientIntVector =
        client->get<std::vector<int>>( clientDocument->demoObject, clientDocument->demoObject->intVector.keyword() );
    ASSERT_EQ( largeIntVector, clientIntVector );

    for ( auto& i : clientIntVector )
    {
        i += 2;
    }
    ASSERT_NE( largeIntVector, clientIntVector );
    client->set( clientDocument->demoObject, clientDocument->demoObject->intVector.keyword(), clientIntVector );

    largeIntVector = serverDocument->demoObject->intVector();
    ASSERT_EQ( largeIntVector, clientIntVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ObjectDeepCopyVsShallowCopy )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );
    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<int> largeIntVector;
    std::mt19937     rng;
    std::generate_n( std::back_inserter( largeIntVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );

    auto clientDemoObjectReference = clientDocument->demoObject.object();
    auto clientDemoObjectClone     = clientDocument->demoObject.deepCloneObject();

    std::string serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument->demoObject );
    CAFFA_DEBUG( serverJson );

    // Stop server *before* we read the client JSON

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );
    thread.join();

    // Should succeed in reading the clone but not the reference
    std::string clientJson;
    ASSERT_NO_THROW( clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectClone.get() ) );
    CAFFA_DEBUG( clientJson );
    ASSERT_EQ( serverJson, clientJson );

    CAFFA_INFO( "Expect errors when trying to write a shallow copied object reference back to the closed server" );
    ASSERT_ANY_THROW( clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectReference ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ObjectDeepCopyFromClient )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );
    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<int> largeIntVector;
    std::mt19937     rng;
    std::generate_n( std::back_inserter( largeIntVector ), 100u, std::ref( rng ) );

    serverDocument->demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );

    auto clientDemoObjectClone = clientDocument->demoObject.deepCloneObject();

    std::string serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument->demoObject() );
    std::string clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectClone.get() );
    ASSERT_EQ( serverJson, clientJson );

    clientDemoObjectClone->intVector = { 1, 2, 3 };

    serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument->demoObject() );
    clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectClone.get() );
    ASSERT_NE( serverJson, clientJson );

    clientDocument->demoObject.deepCopyObjectFrom( clientDemoObjectClone.get() );

    serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument->demoObject() );
    clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectClone.get() );
    ASSERT_EQ( serverJson, clientJson );
    CAFFA_INFO( serverJson );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ObjectDoubleGetterAndSetter )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<double> largeDoubleVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( largeDoubleVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->doubleVector = largeDoubleVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientDoubleVector = client->get<std::vector<double>>( clientDocument->demoObject,
                                                                clientDocument->demoObject->doubleVector.keyword() );
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );

    for ( auto& i : clientDoubleVector )
    {
        i += 2;
    }
    ASSERT_NE( largeDoubleVector, clientDoubleVector );
    client->set<std::vector<double>>( clientDocument->demoObject,
                                      clientDocument->demoObject->doubleVector.keyword(),
                                      clientDoubleVector );

    largeDoubleVector = serverDocument->demoObject->doubleVector();
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ObjectIntegratedGettersAndSetters )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    serverDocument->demoObject->intField              = 10;
    serverDocument->demoObject->intFieldNonScriptable = 12;

    serverDocument->demoObject->enumField = DemoObject::T2;
    ASSERT_EQ( DemoObject::T2, serverDocument->demoObject->enumField() );

    ASSERT_EQ( 10, serverDocument->demoObject->intField() );
    ASSERT_EQ( 12, serverDocument->demoObject->intFieldNonScriptable() );

    std::vector<double> serverVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( serverVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->doubleVector = serverVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_EQ( 10, clientDocument->demoObject->intField() );
    ASSERT_NE( 12, clientDocument->demoObject->intFieldNonScriptable() ); // Should not be equal
    ASSERT_EQ( DemoObject::T2, clientDocument->demoObject->enumField() );
    clientDocument->demoObject->enumField = DemoObject::T3;
    ASSERT_EQ( DemoObject::T3, serverDocument->demoObject->enumField() );

    clientDocument->demoObject->enumField = DemoObject::T3;

    auto clientVector = clientDocument->demoObject->doubleVector();

    ASSERT_EQ( serverVector, clientVector );

    for ( auto& i : clientVector )
    {
        i += 2;
    }
    ASSERT_NE( serverVector, clientVector );
    clientDocument->demoObject->doubleVector = clientVector;

    serverVector = serverDocument->demoObject->doubleVector();
    ASSERT_EQ( serverVector, clientVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, EmptyVectorGettersAndSetters )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    std::vector<double> emptyServerVector;
    serverDocument->demoObject->doubleVector = emptyServerVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    auto clientVector = clientDocument->demoObject->doubleVector();

    ASSERT_EQ( emptyServerVector, clientVector );
    ASSERT_TRUE( clientVector.empty() );

    // Set back non-empty vector
    std::vector<double> serverVector         = { 2.0, 3.0 };
    serverDocument->demoObject->doubleVector = serverVector;

    ASSERT_NE( serverVector, clientVector );
    clientVector = clientDocument->demoObject->doubleVector();
    ASSERT_EQ( serverVector, clientVector );
    clientVector.clear();

    clientDocument->demoObject->doubleVector = clientVector;
    serverVector                             = serverDocument->demoObject->doubleVector();

    ASSERT_EQ( clientVector, serverVector );
    ASSERT_TRUE( serverVector.empty() );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, BoolVectorGettersAndSetters )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    ASSERT_EQ( false, serverDocument->demoObject->boolField() );
    serverDocument->demoObject->boolField = true;
    ASSERT_EQ( true, serverDocument->demoObject->boolField() );

    ASSERT_TRUE( serverDocument->demoObject->boolVector().empty() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_EQ( true, clientDocument->demoObject->boolField() );
    ASSERT_TRUE( clientDocument->demoObject->boolVector().empty() );

    std::vector<bool> clientBoolVector = { true, true };
    clientDocument->demoObject->boolVector.setValue( clientBoolVector );
    ASSERT_EQ( clientBoolVector, clientDocument->demoObject->boolVector() );
    ASSERT_EQ( clientBoolVector, serverDocument->demoObject->boolVector() );

    std::vector<bool> serverBoolVector = { false, true, true, false };
    serverDocument->demoObject->boolVector.setValue( serverBoolVector );
    ASSERT_EQ( clientDocument->demoObject->boolVector(), serverBoolVector );
    ASSERT_EQ( serverDocument->demoObject->boolVector(), serverBoolVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, ChildObjects )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                        "localhost",
                                                        ServerApp::s_port,
                                                        ServerApp::s_clientCertFile,
                                                        ServerApp::s_clientKeyFile,
                                                        ServerApp::s_caCertFile );

    auto session        = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_TRUE( clientDocument->demoObject.object() != nullptr );
    clientDocument->demoObject.clear();
    ASSERT_TRUE( clientDocument->demoObject.object() == nullptr );

    size_t childCount = 12u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    ASSERT_EQ( childCount, clientDocument->m_inheritedDemoObjects.size() );
    serverDocument->m_inheritedDemoObjects.clear();
    ASSERT_EQ( 0u, clientDocument->m_inheritedDemoObjects.size() );

    size_t clientChildCount = 4u;
    for ( size_t i = 0; i < clientChildCount; ++i )
    {
        auto inheritedObject     = std::make_unique<InheritedDemoObj>();
        inheritedObject->m_texts = "whatever test";
        clientDocument->addInheritedObject( std::move( inheritedObject ) );
    }
    ASSERT_EQ( clientChildCount, serverDocument->m_inheritedDemoObjects.size() );
    ASSERT_EQ( clientChildCount, clientDocument->m_inheritedDemoObjects.size() );

    for ( size_t i = 0; i < clientChildCount; ++i )
    {
        ASSERT_EQ( "whatever test", serverDocument->m_inheritedDemoObjects[i]->m_texts() );
    }
    clientDocument->m_inheritedDemoObjects.clear();
    ASSERT_EQ( 0u, serverDocument->m_inheritedDemoObjects.size() );
    ASSERT_EQ( 0u, clientDocument->m_inheritedDemoObjects.size() );

    for ( size_t i = 0; i < clientChildCount; ++i )
    {
        auto inheritedObject     = std::make_unique<InheritedDemoObj>();
        inheritedObject->m_texts = "whatever test";
        clientDocument->addInheritedObject( std::move( inheritedObject ) );
    }
    ASSERT_EQ( clientChildCount, serverDocument->m_inheritedDemoObjects.size() );
    ASSERT_EQ( clientChildCount, clientDocument->m_inheritedDemoObjects.size() );

    {
        auto inheritedObject      = std::make_unique<InheritedDemoObj>();
        inheritedObject->intField = 1113;
        clientDocument->m_inheritedDemoObjects.insert( 2u, std::move( inheritedObject ) );
    }
    ASSERT_EQ( clientChildCount + 1u, serverDocument->m_inheritedDemoObjects.size() );
    ASSERT_EQ( clientChildCount + 1u, clientDocument->m_inheritedDemoObjects.size() );

    CAFFA_INFO( "The server now has a new member with an int value of: "
                << serverDocument->m_inheritedDemoObjects[2]->intField() );
    ASSERT_EQ( 1113, serverDocument->m_inheritedDemoObjects[2]->intField() );

    serverDocument->m_inheritedDemoObjects.clear();
    ASSERT_EQ( 0u, serverDocument->m_inheritedDemoObjects.size() );
    ASSERT_EQ( 0u, clientDocument->m_inheritedDemoObjects.size() );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( GrpcTest, LocalResponseTimeAndDataTransfer )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    {
        auto client = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                            "localhost",
                                                            ServerApp::s_port,
                                                            ServerApp::s_clientCertFile,
                                                            ServerApp::s_clientKeyFile,
                                                            ServerApp::s_caCertFile );

        auto session        = serverApp->getExistingSession( client->sessionUuid() );
        auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument", session.get() ) );
        ASSERT_TRUE( serverDocument );
        CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

        auto objectHandle   = client->document( "testDocument" );
        auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
        ASSERT_TRUE( clientDocument != nullptr );

        {
            serverDocument->demoObject->floatVector = { 42.0f };
            auto start_time                         = std::chrono::system_clock::now();
            auto clientVector                       = clientDocument->demoObject->floatVector();
            auto end_time                           = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
            CAFFA_INFO( "Getting single float vector took " << duration << "µs" );
            ASSERT_EQ( serverDocument->demoObject->floatVector(), clientDocument->demoObject->floatVector() );
        }

        std::vector<float> serverVector;
        std::mt19937       rng;
        size_t             numberOfFloats = 128 * 1024;
        serverVector.reserve( numberOfFloats );
        for ( size_t i = 0; i < numberOfFloats; ++i )
        {
            serverVector.push_back( (float)rng() );
        }

        serverDocument->demoObject->floatVector = serverVector;

        {
            auto   start_time   = std::chrono::system_clock::now();
            auto   clientVector = clientDocument->demoObject->floatVector();
            auto   end_time     = std::chrono::system_clock::now();
            auto   duration = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
            size_t KB       = numberOfFloats * sizeof( float ) / ( 1024u );
            CAFFA_INFO( "Transferred " << numberOfFloats << " floats for a total of " << KB << " KB" );
            CAFFA_INFO( "Time spent: " << duration << "ms" );
            double fps = static_cast<float>( numberOfFloats ) / static_cast<float>( duration ) * 1000;
            CAFFA_INFO( "floats per second: " << fps );
            CAFFA_INFO( "KB per second: " << static_cast<float>( KB ) / static_cast<float>( duration ) * 1000 );
        }

        bool ok = client->stopServer();
        ASSERT_TRUE( ok );
    }
    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}

TEST_F( GrpcTest, MultipleSessions )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    {
        std::unique_ptr<caffa::rpc::Client> client1;

        ASSERT_NO_THROW( client1 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                         "localhost",
                                                                         ServerApp::s_port,
                                                                         ServerApp::s_clientCertFile,
                                                                         ServerApp::s_clientKeyFile,
                                                                         ServerApp::s_caCertFile ) );
        ASSERT_TRUE( client1 );
    }
    std::unique_ptr<caffa::rpc::Client> client2;
    ASSERT_NO_THROW( client2 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client2 );
    client2->stopServer();
    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}

TEST_F( GrpcTest, MultipleConcurrentSessionsShouldBeRefused )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    std::unique_ptr<caffa::rpc::Client> client1, client2;

    ASSERT_NO_THROW( client1 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client1 );

    ASSERT_ANY_THROW( client2 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                      "localhost",
                                                                      ServerApp::s_port,
                                                                      ServerApp::s_clientCertFile,
                                                                      ServerApp::s_clientKeyFile,
                                                                      ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client2 == nullptr );
    CAFFA_INFO( "Failed to create new session as expected" );
    client1->stopServer();
    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}

TEST_F( GrpcTest, AdditionalObservingSessions )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    std::unique_ptr<caffa::rpc::Client> client1, client2, client3;

    ASSERT_NO_THROW( client1 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client1 );

    ASSERT_NO_THROW( client2 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::OBSERVING,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client2 != nullptr );

    ASSERT_NO_THROW( client3 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::OBSERVING,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client3 != nullptr );

    // Close observing clients before stopping server
    client2.reset();
    client3.reset();

    client1->stopServer();

    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}

TEST_F( GrpcTest, MultipleConcurrentSessionsDelayWithoutKeepalive )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    std::unique_ptr<caffa::rpc::Client> client1, client2;

    ASSERT_NO_THROW( client1 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client1 );

    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

    ASSERT_NO_THROW( client2 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client2 );

    client1->destroySession();

    client2->stopServer();
    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}

TEST_F( GrpcTest, MultipleConcurrentSessionsWithKeepalive )
{
    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );
    ASSERT_TRUE( serverApp.get() );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    std::unique_ptr<caffa::rpc::Client> client1, client2;

    ASSERT_NO_THROW( client1 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                     "localhost",
                                                                     ServerApp::s_port,
                                                                     ServerApp::s_clientCertFile,
                                                                     ServerApp::s_clientKeyFile,
                                                                     ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client1 );

    for ( size_t i = 0; i < 10; ++i )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        client1->sendKeepAlive();
    }

    ASSERT_ANY_THROW( client2 = std::make_unique<caffa::rpc::Client>( caffa::Session::Type::REGULAR,
                                                                      "localhost",
                                                                      ServerApp::s_port,
                                                                      ServerApp::s_clientCertFile,
                                                                      ServerApp::s_clientKeyFile,
                                                                      ServerApp::s_caCertFile ) );
    ASSERT_TRUE( client2 == nullptr );

    client1->stopServer();
    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}