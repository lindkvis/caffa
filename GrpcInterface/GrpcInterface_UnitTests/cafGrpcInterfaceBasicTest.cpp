
#include "gtest.h"

#include "Parent.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafException.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcClient.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcServer.h"
#include "cafGrpcServerApplication.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafObjectJsonSerializer.h"
#include "cafValueField.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <thread>
#include <vector>

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    DemoObject()
        : m_proxyDoubleValue( 2.1 )
        , m_proxyIntValue( 7 )
        , m_proxyStringValue( "abba" )
    {
        initField( m_proxyDoubleField, "proxyDoubleField" ).withScripting();
        initField( m_proxyIntField, "proxyIntField" ).withScripting();
        initField( m_proxyStringField, "proxyStringField" ).withScripting();

        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleProxy );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::getDoubleProxy );
        m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

        auto intProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<int>>();
        intProxyAccessor->registerSetMethod( this, &DemoObject::setIntProxy );
        intProxyAccessor->registerGetMethod( this, &DemoObject::getIntProxy );
        m_proxyIntField.setAccessor( std::move( intProxyAccessor ) );

        auto stringProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::string>>();
        stringProxyAccessor->registerSetMethod( this, &DemoObject::setStringProxy );
        stringProxyAccessor->registerGetMethod( this, &DemoObject::getStringProxy );
        m_proxyStringField.setAccessor( std::move( stringProxyAccessor ) );

        initField( doubleVector, "doubleVector" ).withScripting();
        initField( floatVector, "floatVector" ).withScripting();
        initField( intVector, "proxyIntVector" ).withScripting();
        initField( stringVector, "proxyStringVector" ).withScripting();

        auto intVectorProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::vector<int>>>();
        intVectorProxyAccessor->registerSetMethod( this, &DemoObject::setIntVectorProxy );
        intVectorProxyAccessor->registerGetMethod( this, &DemoObject::getIntVectorProxy );
        intVector.setAccessor( std::move( intVectorProxyAccessor ) );

        auto stringVectorProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::vector<std::string>>>();
        stringVectorProxyAccessor->registerSetMethod( this, &DemoObject::setStringVectorProxy );
        stringVectorProxyAccessor->registerGetMethod( this, &DemoObject::getStringVectorProxy );
        stringVector.setAccessor( std::move( stringVectorProxyAccessor ) );

        initField( doubleField, "memberDoubleField" ).withScripting().withDefault( 0.0 );
        initField( intField, "memberIntField" ).withScripting().withDefault( 0 );
        initField( stringField, "memberStringField" ).withScripting().withDefault( "" );
        initField( intFieldNonScriptable, "memberIntFieldNonScriptable" ).withDefault( -1 );

        initField( boolField, "memberBoolField" ).withScripting();
        initField( boolVector, "memberVectorBoolField" ).withScripting();
    }

    ~DemoObject() override {}

    // Fields
    caffa::Field<double>      m_proxyDoubleField;
    caffa::Field<int>         m_proxyIntField;
    caffa::Field<std::string> m_proxyStringField;

    caffa::Field<double>      doubleField;
    caffa::Field<int>         intField;
    caffa::Field<int>         intFieldNonScriptable;
    caffa::Field<std::string> stringField;

    caffa::Field<bool>              boolField;
    caffa::Field<std::vector<bool>> boolVector;

    caffa::Field<std::vector<int>>         intVector;
    caffa::Field<std::vector<std::string>> stringVector;

    caffa::Field<std::vector<double>> doubleVector;
    caffa::Field<std::vector<float>>  floatVector;

private:
    // These are proxy getter/setters and should never be called from client, thus are private
    double getDoubleProxy() const { return m_proxyDoubleValue; }
    void   setDoubleProxy( const double& d ) { m_proxyDoubleValue = d; }

    int  getIntProxy() const { return m_proxyIntValue; }
    void setIntProxy( const int& val ) { m_proxyIntValue = val; }

    std::string getStringProxy() const { return m_proxyStringValue; }
    void        setStringProxy( const std::string& val ) { m_proxyStringValue = val; }

    std::vector<int> getIntVectorProxy() const { return m_proxyIntVector; }
    void             setIntVectorProxy( const std::vector<int>& values ) { m_proxyIntVector = values; }

    std::vector<std::string> getStringVectorProxy() const { return m_proxyStringVector; }
    void setStringVectorProxy( const std::vector<std::string>& values ) { m_proxyStringVector = values; }

    double      m_proxyDoubleValue;
    int         m_proxyIntValue;
    std::string m_proxyStringValue;

    std::vector<int>         m_proxyIntVector;
    std::vector<std::string> m_proxyStringVector;
};

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" );

struct DemoObject_copyObjectResult : public caffa::ObjectMethodResult
{
    CAFFA_HEADER_INIT;

    DemoObject_copyObjectResult() { initField( status, "status" ).withDefault( false ); }

    caffa::Field<bool> status;
};

CAFFA_SOURCE_INIT( DemoObject_copyObjectResult, "DemoObject_copyObjectResult", "Object" );

class DemoObject_copyObject : public caffa::ObjectMethod
{
    CAFFA_HEADER_INIT;

public:
    DemoObject_copyObject( caffa::ObjectHandle*      self,
                           double                    doubleValue = -123.0,
                           int                       intValue    = 42,
                           const std::string&        stringValue = "SomeValue",
                           const std::vector<bool>&  boolVector  = {},
                           const std::vector<int>&   intVector   = {},
                           const std::vector<float>& floatVector = {} )
        : caffa::ObjectMethod( self )
    {
        initField( m_doubleField, "doubleField" ).withDefault( doubleValue );
        initField( m_intField, "intField" ).withDefault( intValue );
        initField( m_stringField, "stringField" ).withDefault( stringValue );

        initField( m_boolVector, "boolVector" ).withDefault( boolVector );
        initField( m_intVector, "intVector" ).withDefault( intVector );
        initField( m_floatVector, "floatVector" ).withDefault( floatVector );
    }
    std::pair<bool, std::unique_ptr<caffa::ObjectMethodResult>> execute() override
    {
        CAFFA_DEBUG( "Executing object method on server with values: " << m_doubleField() << ", " << m_intField()
                                                                       << ", " << m_stringField() );
        gsl::not_null<DemoObject*> demoObject = self<DemoObject>();
        demoObject->doubleField               = m_doubleField;
        demoObject->intField                  = m_intField;
        demoObject->stringField               = m_stringField;
        demoObject->intVector                 = m_intVector;
        demoObject->boolVector                = m_boolVector;
        demoObject->floatVector               = m_floatVector;

        auto demoObjectResult    = std::make_unique<DemoObject_copyObjectResult>();
        demoObjectResult->status = true;
        return std::make_pair( true, std::move( demoObjectResult ) );
    }
    std::unique_ptr<caffa::ObjectMethodResult> defaultResult() const override
    {
        return std::make_unique<DemoObject_copyObjectResult>();
    }

public:
    caffa::Field<double>      m_doubleField;
    caffa::Field<int>         m_intField;
    caffa::Field<std::string> m_stringField;

    caffa::Field<std::vector<bool>>  m_boolVector;
    caffa::Field<std::vector<int>>   m_intVector;
    caffa::Field<std::vector<float>> m_floatVector;
};

CAFFA_OBJECT_METHOD_SOURCE_INIT( DemoObject, DemoObject_copyObject, "DemoObject_copyObject" );

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        initField( m_texts, "Texts" ).withScripting();
        initField( m_childArrayField, "DemoObjectects" ).withScripting();
    }

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};

CAFFA_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject", "DemoObject" );

class DemoDocument : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    DemoDocument()
    {
        initField( m_demoObject, "DemoObject" ).withScripting();
        initField( m_inheritedDemoObjects, "InheritedDemoObjects" ).withScripting();
        m_demoObject = std::make_unique<DemoObject>();

        this->setId( "testDocument" );
        this->setFileName( "dummyFileName" );
    }

    void addInheritedObject( std::unique_ptr<InheritedDemoObj> object )
    {
        m_inheritedDemoObjects.push_back( std::move( object ) );
    }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.value(); }

    caffa::ChildField<DemoObject*>            m_demoObject;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

CAFFA_SOURCE_INIT( DemoDocument, "DemoDocument", "Document", "Object" );

class DemoDocumentWithNonScriptableMember : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    DemoDocumentWithNonScriptableMember()
    {
        initField( m_demoObject, "DemoObject" ).withScripting();
        initField( m_demoObjectNonScriptable, "DemoObjectNonScriptable" );
        initField( m_inheritedDemoObjects, "InheritedDemoObjects" ).withScripting();
        m_demoObject              = std::make_unique<DemoObject>();
        m_demoObjectNonScriptable = std::make_unique<DemoObject>();

        this->setId( "testDocument2" );
        this->setFileName( "dummyFileName2" );
    }

    void addInheritedObject( std::unique_ptr<InheritedDemoObj> object )
    {
        m_inheritedDemoObjects.push_back( std::move( object ) );
    }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.value(); }

    caffa::ChildField<DemoObject*>            m_demoObject;
    caffa::ChildField<DemoObject*>            m_demoObjectNonScriptable;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

CAFFA_SOURCE_INIT( DemoDocumentWithNonScriptableMember, "DemoDocumentNonScriptableMember", "Document", "Object" );

class ServerApp : public caffa::rpc::ServerApplication
{
public:
    ServerApp( int port )
        : caffa::rpc::ServerApplication( port )
        , m_demoDocument( std::make_unique<DemoDocument>() )
        , m_demoDocumentWithNonScriptableMember( std::make_unique<DemoDocumentWithNonScriptableMember>() )
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

    caffa::Document* document( const std::string& documentId ) override
    {
        CAFFA_TRACE( "Looking for " << documentId );
        for ( auto document : documents() )
        {
            CAFFA_TRACE( "Found document: " << document->id() );
            if ( documentId.empty() || documentId == document->id() )
            {
                CAFFA_TRACE( "Match!!" );
                return document;
            }
        }
        return nullptr;
    }
    const caffa::Document* document( const std::string& documentId ) const override
    {
        CAFFA_TRACE( "Looking for " << documentId );
        for ( auto document : documents() )
        {
            CAFFA_TRACE( "Found document: " << document->id() );
            if ( documentId.empty() || documentId == document->id() )
            {
                CAFFA_TRACE( "Match!!" );
                return document;
            }
        }
        return nullptr;
    }
    std::list<caffa::Document*> documents() override
    {
        return { m_demoDocument.get(), m_demoDocumentWithNonScriptableMember.get() };
    }
    std::list<const caffa::Document*> documents() const override
    {
        return { m_demoDocument.get(), m_demoDocumentWithNonScriptableMember.get() };
    }

    void resetToDefaultData() override { m_demoDocument = std::make_unique<DemoDocument>(); }

private:
    void onStartup() override { CAFFA_DEBUG( "Starting Server" ); }
    void onShutdown() override { CAFFA_DEBUG( "Shutting down Server" ); }

private:
    std::unique_ptr<DemoDocument>                        m_demoDocument;
    std::unique_ptr<DemoDocumentWithNonScriptableMember> m_demoDocumentWithNonScriptableMember;
};

TEST( BaseTest, Launch )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    {
        auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );

        // caffa::AppInfo appInfo = client->appInfo();
        // ASSERT_EQ( serverApp->name(), appInfo.name );

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
TEST( BaseTest, Document )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    CAFFA_DEBUG( "Launching Server" );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    CAFFA_DEBUG( "Launching Client" );
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );
    CAFFA_INFO( "Expect failure to get document with the wrong document ID. Next line should be an error." );
    auto nonExistentDocument = dynamic_cast<DemoDocument*>( serverApp->document( "wrongName" ) );
    ASSERT_TRUE( nonExistentDocument == nullptr );
    auto blankNameDocument = dynamic_cast<DemoDocument*>( serverApp->document( "" ) );
    ASSERT_TRUE( blankNameDocument );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    auto nonExistentClientDocument = client->document( "wrongName" );
    ASSERT_TRUE( nonExistentDocument == nullptr );

    try
    {
        auto clientFileName = clientDocument->fileName();
        CAFFA_DEBUG( "Client Document File Name: " << clientFileName );
    }
    catch ( const caffa::Exception& e )
    {
        CAFFA_ERROR( "Exception caught: " << e.what() );
        return;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Exception caught" );
        return;
    }
    ASSERT_EQ( serverApp->document( "testDocument" )->fileName(), clientDocument->fileName() );
    ASSERT_EQ( serverDocument->uuid(), clientDocument->uuid() );

    {
        auto serverDescendants =
            serverDocument->matchingDescendants( []( const caffa::ObjectHandle* objectHandle ) -> bool
                                                 { return dynamic_cast<const DemoObject*>( objectHandle ) != nullptr; } );

        auto clientDescendants =
            clientDocument->matchingDescendants( []( const caffa::ObjectHandle* objectHandle ) -> bool
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
    std::string serverJson = caffa::ObjectJsonSerializer( true ).writeObjectToString( serverDocument );
    CAFFA_DEBUG( serverJson );
    std::string clientJson = caffa::ObjectJsonSerializer( true ).writeObjectToString( clientDocument );
    CAFFA_DEBUG( clientJson );
    ASSERT_EQ( serverJson, clientJson );

    CAFFA_DEBUG( "Confirmed test results!" );
    bool ok = client->stopServer();
    ASSERT_TRUE( ok );
    CAFFA_DEBUG( "Waiting for server thread to join" );
    thread.join();
    CAFFA_DEBUG( "Finishing test" );
}

TEST( BaseTest, DocumentWithNonScriptableChild )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    CAFFA_DEBUG( "Launching Server" );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    CAFFA_DEBUG( "Launching Client" );
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );
    auto serverDocument = dynamic_cast<DemoDocumentWithNonScriptableMember*>( serverApp->document( "testDocument2" ) );
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
        auto serverObject = serverDocument->m_demoObjectNonScriptable();
        auto clientObject = clientDocument->m_demoObjectNonScriptable();

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

    std::string serverJson = caffa::ObjectJsonSerializer( true ).writeObjectToString( serverDocument );
    CAFFA_DEBUG( serverJson );
    std::string clientJson = caffa::ObjectJsonSerializer( true ).writeObjectToString( clientDocument );
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
TEST( BaseTest, Sync )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
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
    ASSERT_EQ( serverApp->document( "testDocument" )->fileName(), clientDocument->fileName() );
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
TEST( BaseTest, ObjectMethod )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
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
    std::string methodKeyword;
    for ( const auto& objectMethod : objectMethods )
    {
        auto ioCapability = objectMethod->capability<caffa::ObjectIoCapability>();
        ASSERT_TRUE( ioCapability );
        CAFFA_TRACE( "Found method: " << ioCapability->classKeyword() );
        methodKeyword = ioCapability->classKeyword();
    }

    std::vector<bool>  boolVector = { true, false, true };
    std::vector<int>   intVector( 10000 );
    std::vector<float> floatVector = { -2.0, 3.0, 4.0, 8.0 };

    std::iota( intVector.begin(), intVector.end(), 0 );

    CAFFA_INFO( "Creating object method" );
    DemoObject_copyObject method( inheritedObjects.front(), 45.3, 43, "AnotherValue", boolVector, intVector, floatVector );
    ASSERT_EQ( method.classKeyword(), methodKeyword );

    CAFFA_INFO( "Execute" );
    auto [result, resultObject] = client->execute( &method );
    ASSERT_TRUE( result );
    ASSERT_TRUE( resultObject != nullptr );
    auto copyObjectResult = dynamic_cast<DemoObject_copyObjectResult*>( resultObject.get() );
    ASSERT_TRUE( copyObjectResult != nullptr );
    ASSERT_EQ( true, copyObjectResult->status() );

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
TEST( BaseTest, ObjectIntGetterAndSetter )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client         = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<int> largeIntVector;
    std::mt19937     rng;
    std::generate_n( std::back_inserter( largeIntVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->intVector = largeIntVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientIntVector =
        client->get<std::vector<int>>( clientDocument->m_demoObject, clientDocument->m_demoObject->intVector.keyword() );
    ASSERT_EQ( largeIntVector, clientIntVector );

    for ( auto& i : clientIntVector )
    {
        i += 2;
    }
    ASSERT_NE( largeIntVector, clientIntVector );
    client->set( clientDocument->m_demoObject, clientDocument->m_demoObject->intVector.keyword(), clientIntVector );

    largeIntVector = serverDocument->m_demoObject->intVector();
    ASSERT_EQ( largeIntVector, clientIntVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ObjectDoubleGetterAndSetter )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<double> largeDoubleVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( largeDoubleVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->doubleVector = largeDoubleVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientDoubleVector = client->get<std::vector<double>>( clientDocument->m_demoObject,
                                                                clientDocument->m_demoObject->doubleVector.keyword() );
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );

    for ( auto& i : clientDoubleVector )
    {
        i += 2;
    }
    ASSERT_NE( largeDoubleVector, clientDoubleVector );
    client->set<std::vector<double>>( clientDocument->m_demoObject,
                                      clientDocument->m_demoObject->doubleVector.keyword(),
                                      clientDoubleVector );

    largeDoubleVector = serverDocument->m_demoObject->doubleVector();
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ObjectIntegratedGettersAndSetters )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    serverDocument->m_demoObject->intField              = 10;
    serverDocument->m_demoObject->intFieldNonScriptable = 12;

    ASSERT_EQ( 10, serverDocument->m_demoObject->intField() );
    ASSERT_EQ( 12, serverDocument->m_demoObject->intFieldNonScriptable() );

    std::vector<double> serverVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( serverVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->doubleVector = serverVector;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_EQ( 10, clientDocument->m_demoObject->intField() );
    ASSERT_NE( 12, clientDocument->m_demoObject->intFieldNonScriptable() ); // Should not be equal

    auto clientVector = clientDocument->m_demoObject->doubleVector();

    ASSERT_EQ( serverVector, clientVector );

    for ( auto& i : clientVector )
    {
        i += 2;
    }
    ASSERT_NE( serverVector, clientVector );
    clientDocument->m_demoObject->doubleVector = clientVector;

    serverVector = serverDocument->m_demoObject->doubleVector();
    ASSERT_EQ( serverVector, clientVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, BoolVectorGettersAndSetters )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    ASSERT_EQ( false, serverDocument->m_demoObject->boolField() );
    serverDocument->m_demoObject->boolField = true;
    ASSERT_EQ( true, serverDocument->m_demoObject->boolField() );

    ASSERT_TRUE( serverDocument->m_demoObject->boolVector().empty() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_EQ( true, clientDocument->m_demoObject->boolField() );
    ASSERT_TRUE( clientDocument->m_demoObject->boolVector().empty() );

    std::vector<bool> clientBoolVector = { true, true };
    clientDocument->m_demoObject->boolVector.setValue( clientBoolVector );
    ASSERT_EQ( clientBoolVector, clientDocument->m_demoObject->boolVector() );
    ASSERT_EQ( clientBoolVector, serverDocument->m_demoObject->boolVector() );

    std::vector<bool> serverBoolVector = { false, true, true, false };
    serverDocument->m_demoObject->boolVector.setValue( serverBoolVector );
    ASSERT_EQ( clientDocument->m_demoObject->boolVector(), serverBoolVector );
    ASSERT_EQ( serverDocument->m_demoObject->boolVector(), serverBoolVector );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildObjects )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    ASSERT_TRUE( clientDocument->m_demoObject.value() != nullptr );
    clientDocument->m_demoObject.clear();
    ASSERT_TRUE( clientDocument->m_demoObject.value() == nullptr );

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
TEST( BaseTest, LocalResponseTimeAndDataTransfer )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caffa::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    {
        auto client = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );

        auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
        ASSERT_TRUE( serverDocument );
        CAFFA_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

        auto objectHandle   = client->document( "testDocument" );
        auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
        ASSERT_TRUE( clientDocument != nullptr );

        {
            serverDocument->m_demoObject->floatVector = { 42.0f };
            auto start_time                           = std::chrono::system_clock::now();
            auto clientVector                         = clientDocument->m_demoObject->floatVector();
            auto end_time                             = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
            CAFFA_INFO( "Getting single float vector took " << duration << "µs" );
            ASSERT_EQ( serverDocument->m_demoObject->floatVector(), clientDocument->m_demoObject->floatVector() );
        }

        std::vector<float> serverVector;
        std::mt19937       rng;
        size_t             numberOfFloats = 1024u * 1024u * 4;
        serverVector.reserve( numberOfFloats );
        for ( size_t i = 0; i < numberOfFloats; ++i )
        {
            serverVector.push_back( (float)rng() );
        }

        serverDocument->m_demoObject->floatVector = serverVector;

        {
            auto   start_time   = std::chrono::system_clock::now();
            auto   clientVector = clientDocument->m_demoObject->floatVector();
            auto   end_time     = std::chrono::system_clock::now();
            auto   duration = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
            size_t MB       = numberOfFloats * sizeof( float ) / ( 1024u * 1024u );
            CAFFA_INFO( "Transferred " << numberOfFloats << " floats for a total of " << MB << " MB" );
            CAFFA_INFO( "Time spent: " << duration << "ms" );
            double fps = static_cast<float>( numberOfFloats ) / static_cast<float>( duration ) * 1000;
            CAFFA_INFO( "floats per second: " << fps );
            CAFFA_INFO( "MB per second: " << static_cast<float>( MB ) / static_cast<float>( duration ) * 1000 );
        }

        bool ok = client->stopServer();
        ASSERT_TRUE( ok );
    }
    CAFFA_DEBUG( "Stopping server and waiting for server to join" );
    thread.join();
    CAFFA_DEBUG( "Server joined" );
}
