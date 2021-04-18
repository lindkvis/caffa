
#include "gtest.h"

#include "Parent.h"

#include "../cafGrpcClient.h"
#include "../cafGrpcClientObjectFactory.h"
#include "../cafGrpcObjectClientCapability.h"
#include "../cafGrpcServer.h"
#include "../cafGrpcServerApplication.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafException.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafPtrField.h"
#include "cafValueField.h"

#include <chrono>
#include <random>
#include <string>
#include <thread>
#include <vector>

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject()
    {
        initField( m_proxyDoubleField, "proxyDoubleField" ).withScripting();
        initField( m_proxyIntField, "proxyIntField" ).withScripting();
        initField( m_proxyStringField, "proxyStringField" ).withScripting();

        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setFieldDataAccessor( std::move( doubleProxyAccessor ) );

        auto intProxyAccessor = std::make_unique<caf::FieldProxyAccessor<int>>();
        intProxyAccessor->registerSetMethod( this, &DemoObject::setIntMember );
        intProxyAccessor->registerGetMethod( this, &DemoObject::intMember );
        m_proxyIntField.setFieldDataAccessor( std::move( intProxyAccessor ) );

        auto stringProxyAccessor = std::make_unique<caf::FieldProxyAccessor<std::string>>();
        stringProxyAccessor->registerSetMethod( this, &DemoObject::setStringMember );
        stringProxyAccessor->registerGetMethod( this, &DemoObject::stringMember );
        m_proxyStringField.setFieldDataAccessor( std::move( stringProxyAccessor ) );

        initField( m_doubleVector, "doubleVector" ).withScripting();
        initField( m_floatVector, "floatVector" ).withScripting();
        initField( m_intVectorProxy, "proxyIntVector" ).withScripting();
        initField( m_stringVectorProxy, "proxyStringVector" ).withScripting();

        auto intVectorProxyAccessor = std::make_unique<caf::FieldProxyAccessor<std::vector<int>>>();
        intVectorProxyAccessor->registerSetMethod( this, &DemoObject::setIntVector );
        intVectorProxyAccessor->registerGetMethod( this, &DemoObject::getIntVector );
        m_intVectorProxy.setFieldDataAccessor( std::move( intVectorProxyAccessor ) );

        auto stringVectorProxyAccessor = std::make_unique<caf::FieldProxyAccessor<std::vector<std::string>>>();
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
    caf::Field<double>      m_proxyDoubleField;
    caf::Field<int>         m_proxyIntField;
    caf::Field<std::string> m_proxyStringField;

    caf::Field<double>      m_memberDoubleField;
    caf::Field<int>         m_memberIntField;
    caf::Field<std::string> m_memberStringField;

    caf::Field<std::vector<int>>         m_intVectorProxy;
    caf::Field<std::vector<std::string>> m_stringVectorProxy;

    caf::Field<std::vector<double>> m_doubleVector;
    caf::Field<std::vector<float>>  m_floatVector;

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

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

struct DemoObject_copyObjectResult : public caf::Object
{
    CAF_HEADER_INIT;

    DemoObject_copyObjectResult() { initField( status, "status" ).withDefault( false ); }

    caf::Field<bool> status;
};

CAF_SOURCE_INIT( DemoObject_copyObjectResult, "DemoObjectResult" );

class DemoObject_copyObject : public caf::ObjectMethod
{
    CAF_HEADER_INIT;

public:
    DemoObject_copyObject( caf::ObjectHandle* self,
                           double             doubleValue = -123.0,
                           int                intValue    = 42,
                           const std::string& stringValue = "SomeValue" )
        : caf::ObjectMethod( self )
    {
        initField( m_doubleMember, "doubleMember" ).withScripting().withDefault( doubleValue );
        initField( m_intMember, "intMember" ).withScripting().withDefault( intValue );
        initField( m_stringMember, "stringMember" ).withScripting().withDefault( stringValue );
    }
    caf::ObjectHandle* execute() override
    {
        CAF_DEBUG( "Executing object method on server" );
        gsl::not_null<DemoObject*> demoObject = self<DemoObject>();
        demoObject->setDoubleMember( m_doubleMember );
        demoObject->setIntMember( m_intMember );
        demoObject->setStringMember( m_stringMember );

        auto demoObjectResult    = std::make_unique<DemoObject_copyObjectResult>();
        demoObjectResult->status = true;
        return demoObjectResult.release();
    }
    bool                          resultIsPersistent() const override { return false; }
    std::unique_ptr<ObjectHandle> defaultResult() const override
    {
        return std::make_unique<DemoObject_copyObjectResult>();
    }

private:
    caf::Field<double>      m_doubleMember;
    caf::Field<int>         m_intMember;
    caf::Field<std::string> m_stringMember;
};

CAF_OBJECT_METHOD_SOURCE_INIT( DemoObject, DemoObject_copyObject, "copyObject" );

class InheritedDemoObj : public DemoObject
{
    CAF_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        this->addField( &m_texts, "Texts" );
        this->addField( &m_childArrayField, "DemoObjectects" );
        this->addField( &m_ptrField, "m_ptrField" );
    }

    caf::Field<std::string>           m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
    caf::PtrField<InheritedDemoObj*>  m_ptrField;
};

CAF_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject", "DemoObject" );

class DemoDocument : public caf::Document
{
    CAF_HEADER_INIT;

public:
    DemoDocument()
    {
        initField( m_demoObject, "DemoObject" );
        initField( m_inheritedDemoObjects, "InheritedDemoObjects" );
        m_demoObject = std::make_unique<DemoObject>();

        this->fileName = "dummyFileName";
    }

    void addInheritedObject( std::unique_ptr<InheritedDemoObj> object )
    {
        m_inheritedDemoObjects.push_back( std::move( object ) );
    }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.childObjects(); }

    caf::ChildField<DemoObject*>            m_demoObject;
    caf::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

CAF_SOURCE_INIT( DemoDocument, "DemoDocument" );

class ServerApp : public caf::rpc::ServerApplication
{
public:
    ServerApp( int port )
        : caf::rpc::ServerApplication( port )
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

    caf::Document*                  document( const std::string& documentId ) override { return &m_demoDocument; }
    const caf::Document*            document( const std::string& documentId ) const override { return &m_demoDocument; }
    std::list<caf::Document*>       documents() override { return { document( "" ) }; }
    std::list<const caf::Document*> documents() const override { return { document( "" ) }; }

private:
    DemoDocument m_demoDocument;
};

TEST( BaseTest, Launch )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    caf::AppInfo appInfo = client->appInfo();
    ASSERT_EQ( serverApp->name(), appInfo.name );

    CAF_DEBUG( "Confirmed test results!" );
    bool ok = client->stopServer();
    ASSERT_TRUE( ok );
    CAF_DEBUG( "Waiting for server thread to join" );
    thread.join();
    CAF_DEBUG( "Finishing test" );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Document )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    CAF_DEBUG( "Launching Server" );
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    CAF_DEBUG( "Launching Client" );
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAF_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<caf::Document*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    try
    {
        auto clientFileName = clientDocument->fileName();
        CAF_DEBUG( "Client Document File Name: " << clientFileName );
    }
    catch ( const caf::Exception& e )
    {
        CAF_ERROR( "Exception caught: " << e.what() );
        return;
    }
    catch ( ... )
    {
        CAF_ERROR( "Exception caught" );
        return;
    }
    ASSERT_EQ( serverApp->document( "testDocument" )->fileName(), clientDocument->fileName() );

    auto clientCapability = clientDocument->capability<caf::rpc::ObjectClientCapability>();
    ASSERT_TRUE( clientCapability != nullptr );
    ASSERT_EQ( reinterpret_cast<uint64_t>( serverDocument ), clientCapability->addressOnServer() );

    auto serverDescendants = serverDocument->matchingDescendants( []( const caf::ObjectHandle* objectHandle ) -> bool {
        return dynamic_cast<const InheritedDemoObj*>( objectHandle ) != nullptr;
    } );

    auto clientDescendants = clientDocument->matchingDescendants( []( const caf::ObjectHandle* objectHandle ) -> bool {
        return dynamic_cast<const InheritedDemoObj*>( objectHandle ) != nullptr;
    } );

    ASSERT_EQ( childCount, serverDescendants.size() );
    ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
    for ( auto server_it = serverDescendants.begin(), client_it = clientDescendants.begin();
          server_it != serverDescendants.end();
          ++server_it, ++client_it )
    {
        auto childClientCapability = ( *client_it )->capability<caf::rpc::ObjectClientCapability>();
        ASSERT_TRUE( childClientCapability != nullptr );
        ASSERT_EQ( reinterpret_cast<uint64_t>( *server_it ), childClientCapability->addressOnServer() );
    }

    CAF_DEBUG( "Confirmed test results!" );
    bool ok = client->stopServer();
    ASSERT_TRUE( ok );
    CAF_DEBUG( "Waiting for server thread to join" );
    thread.join();
    CAF_DEBUG( "Finishing test" );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Sync )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<caf::Document*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    CAF_DEBUG( "Client Document File Name: " << clientDocument->fileName() );
    ASSERT_EQ( serverApp->document( "testDocument" )->fileName(), clientDocument->fileName() );

    auto clientCapability = clientDocument->capability<caf::rpc::ObjectClientCapability>();
    ASSERT_TRUE( clientCapability != nullptr );
    ASSERT_EQ( reinterpret_cast<uint64_t>( serverDocument ), clientCapability->addressOnServer() );

    std::string newFileName  = "ChangedFileName.txt";
    clientDocument->fileName = newFileName;
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

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );

    CAF_DEBUG( "Adding object to server" );
    serverDocument->addInheritedObject( std::make_unique<InheritedDemoObj>() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    auto serverObjects = serverDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, serverObjects.size() );
    CAF_DEBUG( "Getting client objects" );
    auto inheritedObjects = clientDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, inheritedObjects.size() );

    CAF_DEBUG( "Creating object method" );
    DemoObject_copyObject method( inheritedObjects.front(), 45.3, 43, "AnotherValue" );
    CAF_DEBUG( "Execute" );
    auto result = client->execute( &method );
    ASSERT_TRUE( result != nullptr );
    auto copyObjectResult = dynamic_cast<DemoObject_copyObjectResult*>( result.get() );
    ASSERT_TRUE( copyObjectResult && copyObjectResult->status() );

    CAF_DEBUG( "Get double member" );
    ASSERT_EQ( 45.3, serverObjects.front()->doubleMember() );
    CAF_DEBUG( "Get int member" );
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

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAF_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<int> largeIntVector;
    std::mt19937     rng;
    std::generate_n( std::back_inserter( largeIntVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->setIntVector( largeIntVector );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientCapability = clientDocument->capability<caf::rpc::ObjectClientCapability>();
    ASSERT_TRUE( clientCapability != nullptr );
    ASSERT_TRUE( clientDocument->m_demoObject->capability<caf::rpc::ObjectClientCapability>() != nullptr );
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

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAF_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    std::vector<double> largeDoubleVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( largeDoubleVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->setDoubleVector( largeDoubleVector );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientCapability = clientDocument->capability<caf::rpc::ObjectClientCapability>();
    ASSERT_TRUE( clientCapability != nullptr );
    ASSERT_TRUE( clientDocument->m_demoObject->capability<caf::rpc::ObjectClientCapability>() != nullptr );
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

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAF_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

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
TEST( BaseTest, LocalResponseTimeAndDataTransfer )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    CAF_DEBUG( "Server Document File Name: " << serverDocument->fileName() );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    {
        serverDocument->m_demoObject->setFloatVector( { 42.0f } );
        auto start_time   = std::chrono::system_clock::now();
        auto clientVector = clientDocument->m_demoObject->floatVector();
        auto end_time     = std::chrono::system_clock::now();
        auto duration     = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
        CAF_INFO( "Getting single float vector took " << duration << "µs" );
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
        auto   duration     = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        size_t MB           = numberOfFloats * sizeof( float ) / ( 1024u * 1024u );
        CAF_INFO( "Transferred " << numberOfFloats << " floats for a total of " << MB << " MB" );
        CAF_INFO( "Time spent: " << duration << "ms" );
        double fps = static_cast<float>( numberOfFloats ) / static_cast<float>( duration ) * 1000;
        CAF_INFO( "floats per second: " << fps );
        CAF_INFO( "MB per second: " << static_cast<float>( MB ) / static_cast<float>( duration ) * 1000 );
    }

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}
