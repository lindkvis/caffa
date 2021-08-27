
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
#include "cafValueField.h"

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
    {
        initField( m_proxyDoubleField, "proxyDoubleField" ).withScripting();
        initField( m_proxyIntField, "proxyIntField" ).withScripting();
        initField( m_proxyStringField, "proxyStringField" ).withScripting();

        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setFieldDataAccessor( std::move( doubleProxyAccessor ) );

        auto intProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<int>>();
        intProxyAccessor->registerSetMethod( this, &DemoObject::setIntMember );
        intProxyAccessor->registerGetMethod( this, &DemoObject::intMember );
        m_proxyIntField.setFieldDataAccessor( std::move( intProxyAccessor ) );

        auto stringProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::string>>();
        stringProxyAccessor->registerSetMethod( this, &DemoObject::setStringMember );
        stringProxyAccessor->registerGetMethod( this, &DemoObject::stringMember );
        m_proxyStringField.setFieldDataAccessor( std::move( stringProxyAccessor ) );

        initField( m_doubleVector, "doubleVector" ).withScripting();
        initField( m_floatVector, "floatVector" ).withScripting();
        initField( m_intVectorProxy, "proxyIntVector" ).withScripting();
        initField( m_stringVectorProxy, "proxyStringVector" ).withScripting();

        auto intVectorProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::vector<int>>>();
        intVectorProxyAccessor->registerSetMethod( this, &DemoObject::setIntVector );
        intVectorProxyAccessor->registerGetMethod( this, &DemoObject::getIntVector );
        m_intVectorProxy.setFieldDataAccessor( std::move( intVectorProxyAccessor ) );

        auto stringVectorProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::vector<std::string>>>();
        stringVectorProxyAccessor->registerSetMethod( this, &DemoObject::setStringVector );
        stringVectorProxyAccessor->registerGetMethod( this, &DemoObject::getStringVector );
        m_stringVectorProxy.setFieldDataAccessor( std::move( stringVectorProxyAccessor ) );

        initField( m_memberDoubleField, "memberDoubleField" ).withScripting();
        initField( m_memberIntField, "memberIntField" ).withScripting();
        initField( m_memberStringField, "memberStringField" ).withScripting();

        // Default values
        m_doubleMember = 2.1;
        m_intMember    = 7;
        m_stringMember = "abba";

        m_memberDoubleField = 0.0;
        m_memberIntField    = 0;
        m_memberStringField = "";
    }

    ~DemoObject() override {}

    // Fields
    caffa::Field<double>      m_proxyDoubleField;
    caffa::Field<int>         m_proxyIntField;
    caffa::Field<std::string> m_proxyStringField;

    caffa::Field<double>      m_memberDoubleField;
    caffa::Field<int>         m_memberIntField;
    caffa::Field<std::string> m_memberStringField;

    caffa::Field<std::vector<int>>         m_intVectorProxy;
    caffa::Field<std::vector<std::string>> m_stringVectorProxy;

    caffa::Field<std::vector<double>> m_doubleVector;
    caffa::Field<std::vector<float>>  m_floatVector;

    double doubleMember() const { return m_doubleMember; }
    void   setDoubleMember( const double& d ) { m_doubleMember = d; }

    int  intMember() const { return m_intMember; }
    void setIntMember( const int& val ) { m_intMember = val; }

    std::string stringMember() const { return m_stringMember; }
    void        setStringMember( const std::string& val ) { m_stringMember = val; }

    std::vector<double> doubleVector() const { return m_doubleVector; }
    void                setDoubleVector( const std::vector<double>& values ) { m_doubleVector = values; }

    std::vector<float> floatVector() const { return m_floatVector; }
    void               setFloatVector( const std::vector<float>& values ) { m_floatVector = values; }

    std::vector<int> getIntVector() const { return m_intVector; }
    void             setIntVector( const std::vector<int>& values ) { m_intVector = values; }

    std::vector<std::string> getStringVector() const { return m_stringVector; }
    void                     setStringVector( const std::vector<std::string>& values ) { m_stringVector = values; }

private:
    double      m_doubleMember;
    int         m_intMember;
    std::string m_stringMember;

    std::vector<int>         m_intVector;
    std::vector<std::string> m_stringVector;
};

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" );

struct DemoObject_copyObjectResult : public caffa::Object
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
    DemoObject_copyObject( caffa::ObjectHandle* self,
                           double               doubleValue = -123.0,
                           int                  intValue    = 42,
                           const std::string&   stringValue = "SomeValue" )
        : caffa::ObjectMethod( self )
    {
        initField( m_doubleMember, "doubleMember" ).withScripting().withDefault( doubleValue );
        initField( m_intMember, "intMember" ).withScripting().withDefault( intValue );
        initField( m_stringMember, "stringMember" ).withScripting().withDefault( stringValue );
    }
    std::unique_ptr<caffa::ObjectHandle> execute() override
    {
        CAFFA_DEBUG( "Executing object method on server with values: " << m_doubleMember() << ", " << m_intMember()
                                                                       << ", " << m_stringMember() );
        gsl::not_null<DemoObject*> demoObject = self<DemoObject>();
        demoObject->setDoubleMember( m_doubleMember );
        demoObject->setIntMember( m_intMember );
        demoObject->setStringMember( m_stringMember );

        auto demoObjectResult    = std::make_unique<DemoObject_copyObjectResult>();
        demoObjectResult->status = true;
        return demoObjectResult;
    }
    std::unique_ptr<ObjectHandle> defaultResult() const override
    {
        return std::make_unique<DemoObject_copyObjectResult>();
    }

private:
    caffa::Field<double>      m_doubleMember;
    caffa::Field<int>         m_intMember;
    caffa::Field<std::string> m_stringMember;
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
        if ( documentId.empty() || documentId == m_demoDocument.id() )
            return &m_demoDocument;
        else
            return nullptr;
    }
    const caffa::Document* document( const std::string& documentId ) const override
    {
        if ( documentId.empty() || documentId == m_demoDocument.id() )
            return &m_demoDocument;
        else
            return nullptr;
    }
    std::list<caffa::Document*>       documents() override { return { document( "" ) }; }
    std::list<const caffa::Document*> documents() const override { return { document( "" ) }; }

private:
    DemoDocument m_demoDocument;
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
    auto client              = std::make_unique<caffa::rpc::Client>( "localhost", portNumber );
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

    std::string serverJson = serverDocument->writeObjectToString();
    CAFFA_DEBUG( serverJson );
    std::string clientJson = clientDocument->writeObjectToString();
    CAFFA_DEBUG( clientJson );
    ASSERT_EQ( serverJson, clientJson );

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

    CAFFA_DEBUG( "Creating object method" );
    DemoObject_copyObject method( inheritedObjects.front(), 45.3, 43, "AnotherValue" );
    ASSERT_EQ( method.classKeyword(), methodKeyword );

    CAFFA_DEBUG( "Execute" );
    auto result = client->execute( &method );
    ASSERT_TRUE( result != nullptr );
    auto copyObjectResult = dynamic_cast<DemoObject_copyObjectResult*>( result.get() );
    ASSERT_TRUE( copyObjectResult && copyObjectResult->status() );

    CAFFA_DEBUG( "Get double member" );
    ASSERT_EQ( 45.3, serverObjects.front()->doubleMember() );
    CAFFA_DEBUG( "Get int member" );
    ASSERT_EQ( 43, serverObjects.front()->intMember() );
    ASSERT_EQ( "AnotherValue", serverObjects.front()->stringMember() );

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

    serverDocument->m_demoObject->setIntVector( largeIntVector );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientIntVector = client->get<std::vector<int>>( clientDocument->m_demoObject,
                                                          clientDocument->m_demoObject->m_intVectorProxy.keyword() );
    ASSERT_EQ( largeIntVector, clientIntVector );

    for ( auto& i : clientIntVector )
    {
        i += 2;
    }
    ASSERT_NE( largeIntVector, clientIntVector );
    client->set( clientDocument->m_demoObject, clientDocument->m_demoObject->m_intVectorProxy.keyword(), clientIntVector );

    largeIntVector = serverDocument->m_demoObject->getIntVector();
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

    serverDocument->m_demoObject->setDoubleVector( largeDoubleVector );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientDoubleVector = client->get<std::vector<double>>( clientDocument->m_demoObject,
                                                                clientDocument->m_demoObject->m_doubleVector.keyword() );
    ASSERT_EQ( largeDoubleVector, clientDoubleVector );

    for ( auto& i : clientDoubleVector )
    {
        i += 2;
    }
    ASSERT_NE( largeDoubleVector, clientDoubleVector );
    client->set<std::vector<double>>( clientDocument->m_demoObject,
                                      clientDocument->m_demoObject->m_doubleVector.keyword(),
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

    std::vector<double> serverVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( serverVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->setDoubleVector( serverVector );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientVector = clientDocument->m_demoObject->doubleVector();

    ASSERT_EQ( serverVector, clientVector );

    for ( auto& i : clientVector )
    {
        i += 2;
    }
    ASSERT_NE( serverVector, clientVector );
    clientDocument->m_demoObject->setDoubleVector( clientVector );

    serverVector = serverDocument->m_demoObject->doubleVector();
    ASSERT_EQ( serverVector, clientVector );

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
        auto inheritedObject = std::make_unique<InheritedDemoObj>();
        inheritedObject->setIntMember( 1113 );
        clientDocument->m_inheritedDemoObjects.insert( 2u, std::move( inheritedObject ) );
    }
    ASSERT_EQ( clientChildCount + 1u, serverDocument->m_inheritedDemoObjects.size() );
    ASSERT_EQ( clientChildCount + 1u, clientDocument->m_inheritedDemoObjects.size() );

    CAFFA_INFO( "The server now has a new member with an int value of: "
                << serverDocument->m_inheritedDemoObjects[2]->intMember() );
    ASSERT_EQ( 1113, serverDocument->m_inheritedDemoObjects[2]->intMember() );

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
            serverDocument->m_demoObject->setFloatVector( { 42.0f } );
            auto start_time   = std::chrono::system_clock::now();
            auto clientVector = clientDocument->m_demoObject->floatVector();
            auto end_time     = std::chrono::system_clock::now();
            auto duration     = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
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

        serverDocument->m_demoObject->setFloatVector( serverVector );

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
