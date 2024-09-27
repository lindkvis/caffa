
#include "gtest/gtest.h"

#include "Child.h"
#include "DemoObject.h"
#include "ServerApp.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafObjectCollector.h"
#include "cafObjectHandle.h"
#include "cafRestClient.h"
#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafRpcObjectConversion.h"
#include "cafUuidGenerator.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef __APPLE__
using thread = std::thread;
#else
using thread = std::jthread;
#endif

class RestTest : public ::testing::Test
{
protected:
    RestTest()
    {
        auto factory = caffa::rpc::ClientPassByRefObjectFactory::instance();
        factory->registerBasicAccessorCreators<caffa::AppEnum<DemoObject::TestEnumType>>();
#ifndef NDEBUG
        caffa::UuidGenerator::s_dummyUuidCounter = 0u;
#endif

        serverApp = std::make_unique<ServerApp>( ServerApp::s_port, 1 );
    }

    void SetUp() override
    {
        CAFFA_DEBUG( "Launching Server" );
        serverThread = thread( &ServerApp::run, serverApp.get() );

        while ( !serverApp->running() )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
    }

    void TearDown() override
    {
        if ( serverThread.joinable() )
        {
            serverApp->quit();
            CAFFA_DEBUG( "Waiting for server thread to join" );
            serverThread.join();
        }
        CAFFA_DEBUG( "Finishing test" );
    }

    std::unique_ptr<ServerApp> serverApp;
    thread                     serverThread;
};

TEST_F( RestTest, Launch )
{
    ASSERT_TRUE( caffa::rpc::RestServerApplication::instance() != nullptr );

    CAFFA_DEBUG( "Connecting client to server" );
    try
    {
        auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
        ASSERT_TRUE( client->isReady( caffa::Session::Type::REGULAR ) );
        client->createSession( caffa::Session::Type::REGULAR );

        caffa::AppInfo appInfo = client->appInfo();
        ASSERT_EQ( serverApp->name(), appInfo.name );

        CAFFA_DEBUG( "Confirmed test results!" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Exception when connecting to client: " << e.what() );
        FAIL();
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, Document )
{
    ASSERT_TRUE( caffa::rpc::RestServerApplication::instance() != nullptr );

    CAFFA_DEBUG( "Launching Client" );

    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_TRUE( client->isReady( caffa::Session::Type::REGULAR ) );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    CAFFA_INFO( "Expect failure to get document with the wrong document ID. Next line should be an error." );
    auto nonExistentDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "wrongName", session.get() ) );
    ASSERT_TRUE( nonExistentDocument == nullptr );
    auto blankNameDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "", session.get() ) );
    ASSERT_TRUE( blankNameDocument );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        auto child = std::make_shared<InheritedDemoObj>();
        CAFFA_DEBUG( "Added inherited object to server: " << child->uuid() );
        serverDocument->addInheritedObject( child );
    }

    CAFFA_DEBUG( "Now getting client document" );

    try
    {
        std::shared_ptr<caffa::ObjectHandle> objectHandle;
        ASSERT_NO_THROW( objectHandle = client->document( "testDocument" ) );
        ASSERT_TRUE( objectHandle != nullptr );
        auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
        ASSERT_TRUE( clientDocument != nullptr );

        ASSERT_ANY_THROW( auto nonExistentClientDocument = client->document( "wrongName" ) );

        ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

        CAFFA_DEBUG( "Collecting both client and server objects" );

        {
            caffa::ConstObjectCollector<DemoObject> serverCollector, clientCollector;
            serverDocument->accept( &serverCollector );
            clientDocument->accept( &clientCollector );

            auto serverDescendants = serverCollector.objects();
            auto clientDescendants = clientCollector.objects();
            ASSERT_TRUE( !serverDescendants.empty() );
            ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
            for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
                  server_it != serverDescendants.end();
                  ++server_it, ++client_it )
            {
                const auto& serverObject = *server_it;
                const auto& clientObject = *client_it;
                ASSERT_TRUE( serverObject && clientObject );
                ASSERT_EQ( serverObject->uuid(), clientObject->uuid() );
            }
        }

        CAFFA_DEBUG( "Collecting both client and server inherited objects" );

        {
            caffa::ConstObjectCollector<InheritedDemoObj> serverCollector, clientCollector;
            serverDocument->accept( &serverCollector );
            clientDocument->accept( &clientCollector );

            auto serverDescendants = serverCollector.objects();
            auto clientDescendants = clientCollector.objects();

            ASSERT_EQ( childCount, serverDescendants.size() );
            ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
            for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
                  server_it != serverDescendants.end();
                  ++server_it, ++client_it )
            {
                ASSERT_EQ( ( *server_it )->uuid(), ( *client_it )->uuid() );
            }
        }

        CAFFA_DEBUG( "Serializing" );

        caffa::JsonSerializer serverSerializer;
        // Need to only write scriptable field into both client and Server-JSON, otherwise they won't match
        serverSerializer.setFieldSelector( caffa::rpc::fieldIsScriptReadable );
        std::string serverJson = serverSerializer.writeObjectToString( serverDocument.get() );
        CAFFA_DEBUG( serverJson );

        CAFFA_INFO( "Writing client JSON" );
        std::string clientJson = caffa::JsonSerializer()
                                     .setFieldSelector( caffa::rpc::fieldIsScriptReadable )
                                     .writeObjectToString( clientDocument.get() );
        CAFFA_DEBUG( clientJson );
        ASSERT_EQ( serverJson, clientJson );

        CAFFA_DEBUG( "Confirmed test results!" );
    }
    catch ( const std::runtime_error& e )
    {
        CAFFA_ERROR( "Exception caught: " << e.what() );
        FAIL();
        return;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Exception caught" );
        FAIL();
        return;
    }
}

TEST_F( RestTest, DocumentWithNonScriptableChild )
{
    ASSERT_TRUE( caffa::rpc::RestServerApplication::instance() != nullptr );

    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );

    try
    {
        client->createSession( caffa::Session::Type::REGULAR );

        auto session = serverApp->getExistingSession( client->sessionUuid() );

        auto serverDocument = std::dynamic_pointer_cast<DemoDocumentWithNonScriptableMember>(
            serverApp->document( "testDocument2", session.get() ) );
        ASSERT_TRUE( serverDocument );

        auto objectHandle   = client->document( "testDocument2" );
        auto clientDocument = std::dynamic_pointer_cast<DemoDocumentWithNonScriptableMember>( objectHandle );
        ASSERT_TRUE( clientDocument != nullptr );

        ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

        size_t childCount = 11u;
        for ( size_t i = 0; i < childCount; ++i )
        {
            serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
        }

        {
            auto serverObject = serverDocument->demoObjectNonScriptable();
            // The objects are not scriptable, so client object should not exist!
            try
            {
                CAFFA_INFO( "Trying to read non-scriptable object from client. This should fail." );
                auto clientObject = clientDocument->demoObjectNonScriptable();
                FAIL();
                ASSERT_TRUE( clientObject == nullptr );
            }
            catch ( const std::exception& )
            {
                CAFFA_INFO( "Expected exception when trying to access non-scriptable client object" );
            }
        }

        {
            caffa::ConstObjectCollector<InheritedDemoObj> serverCollector, clientCollector;
            serverDocument->accept( &serverCollector );
            clientDocument->accept( &clientCollector );

            auto serverDescendants = serverCollector.objects();
            auto clientDescendants = clientCollector.objects();

            ASSERT_EQ( childCount, serverDescendants.size() );
            ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
            for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
                  server_it != serverDescendants.end();
                  ++server_it, ++client_it )
            {
                ASSERT_EQ( ( *server_it )->uuid(), ( *client_it )->uuid() );
            }
        }

        std::string serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument.get() );
        CAFFA_DEBUG( serverJson );
        std::string clientJson = caffa::JsonSerializer().writeObjectToString( clientDocument.get() );
        CAFFA_DEBUG( clientJson );
        ASSERT_NE( serverJson, clientJson );
        CAFFA_DEBUG( "Confirmed test results!" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Something went wrong in the tests: " << e.what() );
    }
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, Sync )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<caffa::Document>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );
    ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, SettingValueWithObserver )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::OBSERVING );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<caffa::Document>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );
    ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ObjectMethod )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    CAFFA_DEBUG( "Adding object to server" );
    serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );

    auto serverObjects = serverDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, serverObjects.size() );
    CAFFA_DEBUG( "Getting client objects" );
    auto inheritedObjects = clientDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, inheritedObjects.size() );

    CAFFA_INFO( "Execute remote method" );
    inheritedObjects.front()->copyValues( 43, 45.3, "AnotherValue" );

    CAFFA_DEBUG( "Get double member" );
    ASSERT_EQ( 45.3, serverObjects.front()->doubleField() );
    CAFFA_DEBUG( "Get int member" );
    ASSERT_EQ( 43, serverObjects.front()->intField() );
    ASSERT_EQ( "AnotherValue", serverObjects.front()->stringField() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ObjectIntGetterAndSetter )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    std::vector<int> largeIntVector;
    std::mt19937     rng; // NOLINT
    std::generate_n( std::back_inserter( largeIntVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientIntVector = client->get<std::vector<int>>( clientDocument->demoObject().get(),
                                                          clientDocument->demoObject->intVector.keyword() );
    ASSERT_EQ( largeIntVector, clientIntVector );

    for ( auto& i : clientIntVector )
    {
        i += 2;
    }
    ASSERT_NE( largeIntVector, clientIntVector );
    client->set( clientDocument->demoObject().get(), clientDocument->demoObject->intVector.keyword(), clientIntVector );

    largeIntVector = serverDocument->demoObject->intVector();
    ASSERT_EQ( largeIntVector, clientIntVector );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ObjectIntGetterAndSetterBenchmark )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument =
        std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    std::vector<int> largeIntVector;
    std::mt19937     rng;
    std::generate_n( std::back_inserter( largeIntVector ), 512 * 1024u, std::ref( rng ) );

    serverDocument->demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );

    std::vector<int> clientIntVector;

    {
        const auto start_time = std::chrono::steady_clock::now();
        clientIntVector       = client->get<std::vector<int>>( clientDocument->demoObject().get(),
                                                         clientDocument->demoObject->intVector.keyword() );
        const auto end_time   = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        const size_t KB = clientIntVector.size() * sizeof( float ) / ( 1024u );
        CAFFA_INFO( "Received " << clientIntVector.size() << " integers for a total of " << KB << " KB" );
        CAFFA_INFO( "Time spent: " << duration << "ms" );
        CAFFA_INFO( "KB per second: " << static_cast<float>( KB ) / static_cast<float>( duration ) * 1000 );
    }
    ASSERT_EQ( largeIntVector, clientIntVector );

    for ( auto& i : clientIntVector )
    {
        i += 2;
    }
    ASSERT_NE( largeIntVector, clientIntVector );
    {
        const auto start_time = std::chrono::steady_clock::now();
        client->set( clientDocument->demoObject().get(),
                     clientDocument->demoObject->intVector.keyword(),
                     clientIntVector );
        const auto end_time = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        const size_t KB = clientIntVector.size() * sizeof( float ) / ( 1024u );
        CAFFA_INFO( "Sent " << clientIntVector.size() << " integers for a total of " << KB << " KB" );
        CAFFA_INFO( "Time spent: " << duration << "ms" );
        CAFFA_INFO( "KB per second: " << static_cast<float>( KB ) / static_cast<float>( duration ) * 1000 );
    }
    largeIntVector = serverDocument->demoObject->intVector();
    ASSERT_EQ( largeIntVector, clientIntVector );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ObjectDeepCopyVsShallowCopy )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    std::vector<int> largeIntVector;
    std::mt19937     rng; // NOLINT
    std::generate_n( std::back_inserter( largeIntVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );

    auto clientDemoObjectReference = clientDocument->demoObject.object();

    std::shared_ptr<DemoObject> clientDemoObjectClone;
    ASSERT_NO_THROW( clientDemoObjectClone = caffa::JsonSerializer().cloneObject( clientDemoObjectReference.get() ) );

    std::string serverJson = caffa::JsonSerializer().writeObjectToString( serverDocument->demoObject().get() );
    CAFFA_TRACE( serverJson );

    // Stop server *before* we read the client JSON
    serverApp->quit();
    serverThread.join();
    // Should succeed in reading the clone but not the reference
    std::string clientJson;
    ASSERT_NO_THROW( clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectClone.get() ) );
    CAFFA_TRACE( clientJson );
    ASSERT_EQ( serverJson, clientJson );

    CAFFA_INFO( "Expect errors when trying to write a shallow copied object reference after the server is closed" );
    ASSERT_ANY_THROW( clientJson = caffa::JsonSerializer().writeObjectToString( clientDemoObjectReference.get() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ObjectDoubleGetterAndSetter )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    std::vector<double> largeDoubleVector;
    std::mt19937        rng; // NOLINT
    std::generate_n( std::back_inserter( largeDoubleVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->doubleVector = largeDoubleVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientDoubleVector = client->get<std::vector<double>>( clientDocument->demoObject().get(),
                                                                clientDocument->demoObject->doubleVector.keyword() );
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );

    for ( auto& i : clientDoubleVector )
    {
        i += 2;
    }
    ASSERT_NE( largeDoubleVector, clientDoubleVector );
    client->set<std::vector<double>>( clientDocument->demoObject().get(),
                                      clientDocument->demoObject->doubleVector.keyword(),
                                      clientDoubleVector );

    largeDoubleVector = serverDocument->demoObject->doubleVector();
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ObjectIntegratedGettersAndSetters )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

        serverDocument->demoObject->intField              = 10;
        serverDocument->demoObject->intFieldNonScriptable = 12;

    serverDocument->demoObject->enumField = DemoObject::T2;
    ASSERT_EQ( DemoObject::T2, serverDocument->demoObject->enumField() );

    ASSERT_EQ( 10, serverDocument->demoObject->intField() );
    ASSERT_EQ( 12, serverDocument->demoObject->intFieldNonScriptable() );

    std::vector<double> serverVector;
    std::mt19937        rng; // NOLINT
    std::generate_n( std::back_inserter( serverVector ), 10000u, std::ref( rng ) );

    serverDocument->demoObject->doubleVector = serverVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_EQ( 10, clientDocument->demoObject->intField() );
    try
    {
        auto value = clientDocument->demoObject->intFieldNonScriptable();
        FAIL() << "Getting the field value succeeded even though it should have failed: " << value;
    }
    catch ( const std::exception& e )
    {
        SUCCEED() << "Expected exception since the field is not scriptable: " << e.what();
    }
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, EmptyVectorGettersAndSetters )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    std::vector<double> emptyServerVector;
    serverDocument->demoObject->doubleVector = emptyServerVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, BoolVectorGettersAndSetters )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    ASSERT_EQ( false, serverDocument->demoObject->boolField() );
    serverDocument->demoObject->boolField = true;
    ASSERT_EQ( true, serverDocument->demoObject->boolField() );

    ASSERT_TRUE( serverDocument->demoObject->boolVector().empty() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, maps )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument = std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    ASSERT_TRUE( serverDocument->demoObject->floatMap().empty() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_TRUE( clientDocument->demoObject->floatMap().empty() );

    CAFFA_INFO( "Setting floatmap!" );
    std::map<std::string, float> floatMap1 = { { "a", 1.0 }, { "b", 2.0 }, { "c", 3.0 } };
    ASSERT_NO_THROW( serverDocument->demoObject->floatMap = floatMap1 );
    ASSERT_EQ( floatMap1, serverDocument->demoObject->floatMap() );
    ASSERT_EQ( floatMap1, clientDocument->demoObject->floatMap() );

    std::map<std::string, float> floatMap2 = { { "c", 4.0 }, { "a", 2.0 }, { "b", 3.0 } }; // input in weird order

    clientDocument->demoObject->floatMap.setValue( floatMap2 );
    ASSERT_EQ( floatMap2, serverDocument->demoObject->floatMap() );
    ASSERT_EQ( floatMap2, clientDocument->demoObject->floatMap() );

    std::map<std::string, std::string> stringMap = { { "key", "value" }, { "anotherKey", "anotherValue" } };
    clientDocument->demoObject->stringMap.setValue( stringMap );
    ASSERT_EQ( stringMap, serverDocument->demoObject->stringMap() );
    ASSERT_EQ( stringMap, clientDocument->demoObject->stringMap() );

    std::map<std::string, caffa::AppEnum<DemoObject::TestEnumType>> enumMap =
        { { "firstEnum", caffa::AppEnum<DemoObject::TestEnumType>( DemoObject::T1 ) },
          { "secondEnum", caffa::AppEnum<DemoObject::TestEnumType>( DemoObject::T2 ) },
          { "thirdEnum", caffa::AppEnum<DemoObject::TestEnumType>( DemoObject::T1 ) } };
    clientDocument->demoObject->enumMap.setValue( enumMap );
    ASSERT_EQ( enumMap, serverDocument->demoObject->enumMap() );
    ASSERT_EQ( enumMap, clientDocument->demoObject->enumMap() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, ChildObjects )
{
    try
    {
        auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
        client->createSession( caffa::Session::Type::REGULAR );

        auto session = serverApp->getExistingSession( client->sessionUuid() );
        auto serverDocument =
            std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
        ASSERT_TRUE( serverDocument );

        auto objectHandle   = client->document( "testDocument" );
        auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
        ASSERT_TRUE( clientDocument != nullptr );

        ASSERT_TRUE( clientDocument->demoObject.object() != nullptr );
        clientDocument->demoObject.clear();
        ASSERT_TRUE( clientDocument->demoObject.object() == nullptr );

        size_t childCount = 12u;
        for ( size_t i = 0; i < childCount; ++i )
        {
            serverDocument->addInheritedObject( std::make_shared<InheritedDemoObj>() );
        }

        CAFFA_INFO( "Added lots of children" );

        ASSERT_EQ( childCount, clientDocument->m_inheritedDemoObjects.size() );
        serverDocument->m_inheritedDemoObjects.clear();
        ASSERT_EQ( 0u, clientDocument->m_inheritedDemoObjects.size() );

        size_t clientChildCount = 4u;
        for ( size_t i = 0; i < clientChildCount; ++i )
        {
            auto inheritedObject     = std::make_shared<InheritedDemoObj>();
            inheritedObject->m_texts = "whatever test";
            clientDocument->addInheritedObject( inheritedObject );
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
            auto inheritedObject     = std::make_shared<InheritedDemoObj>();
            inheritedObject->m_texts = "whatever test";
            clientDocument->addInheritedObject( inheritedObject );
        }
        ASSERT_EQ( clientChildCount, serverDocument->m_inheritedDemoObjects.size() );
        ASSERT_EQ( clientChildCount, clientDocument->m_inheritedDemoObjects.size() );

        {
            auto inheritedObject      = std::make_shared<InheritedDemoObj>();
            inheritedObject->intField = 1113;
            ASSERT_THROW( inheritedObject->intField = 10000, std::exception );
            clientDocument->m_inheritedDemoObjects.insert( 2u, inheritedObject );
        }
        ASSERT_EQ( clientChildCount + 1u, serverDocument->m_inheritedDemoObjects.size() );
        ASSERT_EQ( clientChildCount + 1u, clientDocument->m_inheritedDemoObjects.size() );

        CAFFA_INFO( "The server now has a new member with an int value of: "
                    << serverDocument->m_inheritedDemoObjects[2]->intField() );
        ASSERT_EQ( 1113, serverDocument->m_inheritedDemoObjects[2]->intField() );

        serverDocument->m_inheritedDemoObjects.clear();
        ASSERT_EQ( 0u, serverDocument->m_inheritedDemoObjects.size() );
        ASSERT_EQ( 0u, clientDocument->m_inheritedDemoObjects.size() );
        CAFFA_DEBUG( "Completed test" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_CRITICAL( "Exception caught in test: " << e.what() );
        FAIL();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RestTest, LocalResponseTimeAndDataTransfer )
{
    auto client = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    client->createSession( caffa::Session::Type::REGULAR );

    auto session = serverApp->getExistingSession( client->sessionUuid() );
    auto serverDocument =
        std::dynamic_pointer_cast<DemoDocument>( serverApp->document( "testDocument", session.get() ) );
    ASSERT_TRUE( serverDocument );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = std::dynamic_pointer_cast<DemoDocument>( objectHandle );
    ASSERT_TRUE( clientDocument != nullptr );

    {
        serverDocument->demoObject->floatVector = { 42.0f };
        auto start_time                         = std::chrono::steady_clock::now();
        auto clientVector                       = clientDocument->demoObject->floatVector();
        auto end_time                           = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
        CAFFA_INFO( "Getting single float vector took " << duration << "µs" );
        ASSERT_EQ( serverDocument->demoObject->floatVector(), clientDocument->demoObject->floatVector() );
    }

    std::vector<float> serverVector;
    std::mt19937       rng;
    size_t             numberOfFloats = 512 * 1024;
    serverVector.reserve( numberOfFloats );
    for ( size_t i = 0; i < numberOfFloats; ++i )
    {
        serverVector.push_back( (float)rng() );
    }

    serverDocument->demoObject->floatVector = serverVector;

    {
        auto   start_time   = std::chrono::steady_clock::now();
        auto   clientVector = clientDocument->demoObject->floatVector();
        auto   end_time     = std::chrono::steady_clock::now();
        auto   duration = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        size_t KB       = numberOfFloats * sizeof( float ) / ( 1024u );
        CAFFA_INFO( "Transferred " << numberOfFloats << " floats for a total of " << KB << " KB" );
        CAFFA_INFO( "Time spent: " << duration << "ms" );
        CAFFA_INFO( "KB per second: " << static_cast<float>( KB ) / static_cast<float>( duration ) * 1000 );
    }
}

TEST_F( RestTest, MultipleSessions )
{
    {
        const auto client1 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
        ASSERT_NO_THROW( client1->createSession( caffa::Session::Type::REGULAR ) );

        ASSERT_TRUE( client1 );
    }
    {
        const auto client2 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
        ASSERT_NO_THROW( client2->createSession( caffa::Session::Type::REGULAR ) );
        ASSERT_TRUE( client2 );
    }
}

TEST_F( RestTest, MultipleConcurrentSessionsShouldBeRefused )
{
    const auto client1 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_TRUE( client1->isReady( caffa::Session::Type::REGULAR ) );
    ASSERT_NO_THROW( client1->createSession( caffa::Session::Type::REGULAR ) );

    const auto client2 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_FALSE( client2->isReady( caffa::Session::Type::REGULAR ) );
    ASSERT_ANY_THROW( client2->createSession( caffa::Session::Type::REGULAR ) );
    CAFFA_INFO( "Failed to create new session as expected" );
}

TEST_F( RestTest, AdditionalObservingSessions )
{
    const auto client1 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_TRUE( client1->isReady( caffa::Session::Type::REGULAR ) );
    ASSERT_NO_THROW( client1->createSession( caffa::Session::Type::REGULAR ) );

    const auto client2 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_TRUE( client2->isReady( caffa::Session::Type::OBSERVING ) );
    ASSERT_NO_THROW( client2->createSession( caffa::Session::Type::OBSERVING ) );

    const auto client3 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_NO_THROW( client3->createSession( caffa::Session::Type::OBSERVING ) );
}

TEST_F( RestTest, MultipleConcurrentSessionsDelayWithoutKeepalive )
{
    const auto client1 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_TRUE( client1->isReady( caffa::Session::Type::REGULAR ) );
    ASSERT_NO_THROW( client1->createSession( caffa::Session::Type::REGULAR ) );

    std::this_thread::sleep_for( std::chrono::milliseconds( 6000 ) );

    const auto client2 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    ASSERT_TRUE( client2->isReady( caffa::Session::Type::REGULAR ) );
    ASSERT_NO_THROW( client2->createSession( caffa::Session::Type::REGULAR ) );

    ASSERT_NO_THROW( client1->destroySession() );
}

TEST_F( RestTest, MultipleConcurrentSessionsWithKeepalive )
{
    const auto client1 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
    try
    {
        ASSERT_TRUE( client1->isReady( caffa::Session::Type::REGULAR ) );
        ASSERT_NO_THROW( client1->createSession( caffa::Session::Type::REGULAR ) );

        for ( size_t i = 0; i < 10; ++i )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            client1->sendKeepAlive();
        }

        CAFFA_INFO( "Expecting errors when creating a new session, because the old one has been kept alive!" );
        const auto client2 = std::make_unique<caffa::rpc::RestClient>( "localhost", ServerApp::s_port );
        ASSERT_FALSE( client2->isReady( caffa::Session::Type::REGULAR ) );
        ASSERT_ANY_THROW( client2->createSession( caffa::Session::Type::REGULAR ) );
        CAFFA_DEBUG( "Test completed" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_CRITICAL( "Exception thrown in test: " << e.what() );
        FAIL();
    }
}
