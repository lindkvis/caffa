
#include "gtest/gtest.h"

#include "Parent.h"

#include "../cafGrpcClient.h"
#include "../cafGrpcClientObjectFactory.h"
#include "../cafGrpcObjectClientCapability.h"
#include "../cafGrpcServer.h"
#include "../cafGrpcServerApplication.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectScriptingCapability.h"
#include "cafPdmDocument.h"
#include "cafPdmReferenceHelper.h"
#include "cafProxyValueField.h"
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
        CAF_InitObject( "Demo Object", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_proxyDoubleField, "proxyDoubleField", "", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_proxyIntField, "proxyIntField", "", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_proxyStringField, "proxyStringField", "", "", "", "" );

        m_proxyDoubleField.registerSetMethod( this, &DemoObject::setDoubleMember );
        m_proxyDoubleField.registerGetMethod( this, &DemoObject::doubleMember );

        m_proxyIntField.registerSetMethod( this, &DemoObject::setIntMember );
        m_proxyIntField.registerGetMethod( this, &DemoObject::intMember );

        m_proxyStringField.registerSetMethod( this, &DemoObject::setStringMember );
        m_proxyStringField.registerGetMethod( this, &DemoObject::stringMember );

        CAF_InitScriptableFieldNoDefault( &m_doubleVector, "doubleVector", "", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_floatVector, "floatVector", "", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_intVectorProxy, "proxyIntVector", "", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_stringVectorProxy, "proxyStringVector", "", "", "", "" );

        m_intVectorProxy.registerGetMethod( this, &DemoObject::getIntVector );
        m_intVectorProxy.registerSetMethod( this, &DemoObject::setIntVector );
        m_stringVectorProxy.registerGetMethod( this, &DemoObject::getStringVector );
        m_stringVectorProxy.registerSetMethod( this, &DemoObject::setStringVector );

        this->addField( &m_memberDoubleField, "m_memberDoubleField" );
        this->addField( &m_memberIntField, "m_memberIntField" );
        this->addField( &m_memberStringField, "m_memberStringField" );

        // Default values
        m_doubleMember = 2.1;
        m_intMember    = 7;
        m_stringMember = "abba";

        m_memberDoubleField = 0.0;
        m_memberIntField    = 0;
        m_memberStringField = "";
    }

    ~DemoObject() {}

    // Fields
    caf::ProxyValueField<double>      m_proxyDoubleField;
    caf::ProxyValueField<int>         m_proxyIntField;
    caf::ProxyValueField<std::string> m_proxyStringField;

    caf::Field<double>      m_memberDoubleField;
    caf::Field<int>         m_memberIntField;
    caf::Field<std::string> m_memberStringField;

    caf::ProxyValueField<std::vector<int>>         m_intVectorProxy;
    caf::ProxyValueField<std::vector<std::string>> m_stringVectorProxy;

    caf::Field<std::vector<double>> m_doubleVector;
    caf::Field<std::vector<float>>  m_floatVector;

    // Internal class members accessed by proxy fields
    double doubleMember() const { return m_doubleMember; }
    void   setDoubleMember( const double& d ) { m_doubleMember = d; }

    int  intMember() const { return m_intMember; }
    void setIntMember( const int& val ) { m_intMember = val; }

    std::string stringMember() const { return m_stringMember; }
    void        setStringMember( const std::string& val ) { m_stringMember = val; }

    std::vector<double> getDoubleVector() const { return m_doubleVector; }
    void                setDoubleVector( const std::vector<double>& values ) { m_doubleVector = values; }

    std::vector<float> getFloatVector() const { return m_floatVector; }
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

    DemoObject_copyObjectResult() { CAF_InitField( &status, "status", false, "", "", "", "" ); }
    virtual ~DemoObject_copyObjectResult() = default;

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
        CAF_InitObject( "Copy values into object", "", "", "Copy all values into the DemoObject" );
        CAF_InitScriptableField( &m_doubleMember, "doubleMember", doubleValue, "", "", "", "" );
        CAF_InitScriptableField( &m_intMember, "intMember", intValue, "", "", "", "" );
        CAF_InitScriptableField( &m_stringMember, "stringMember", stringValue, "", "", "", "" );
    }
    caf::ObjectHandle* execute() override
    {
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
        CAF_InitObject( "Inherited Demo Object", "", "", "" );
        this->addField( &m_texts, "Texts" );
        this->addField( &m_childArrayField, "DemoObjectects" );
        this->addField( &m_ptrField, "m_ptrField" );

    }

    caf::Field<std::string>           m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
    caf::PtrField<InheritedDemoObj*>  m_ptrField;
};

CAF_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject" );

class DemoDocument : public caf::PdmDocument
{
    CAF_HEADER_INIT;

public:
    DemoDocument()
    {
        CAF_InitScriptableObject( "DemoDocument", "", "Demo Document", "" );
        CAF_InitFieldNoDefault( &m_demoObject, "DemoObject", "", "", "", "" );
        CAF_InitFieldNoDefault( &m_inheritedDemoObjects, "InheritedDemoObject", "", "", "", "" );
        m_demoObject = new DemoObject;

        this->fileName = "dummyFileName";
    }

    void addInheritedObject( InheritedDemoObj* object ) { m_inheritedDemoObjects.push_back( object ); }
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

    caf::PdmDocument*            document( const std::string& documentId ) override { return &m_demoDocument; }
    const caf::PdmDocument*      document( const std::string& documentId ) const override { return &m_demoDocument; }
    std::list<caf::PdmDocument*> documents() override { return { document( "" ) }; }
    std::list<const caf::PdmDocument*> documents() const override { return { document( "" ) }; }

private:
    DemoDocument m_demoDocument;
};

TEST( BaseTest, Launch )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    caf::AppInfo appInfo = client->appInfo();
    ASSERT_EQ( serverApp->name(), appInfo.name );

    std::cout << "Confirmed test results" << std::endl;
    bool ok = client->stopServer();
    ASSERT_TRUE( ok );
    std::cout << "Waiting for server thread to join" << std::endl;
    thread.join();
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Document )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( new InheritedDemoObj );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<caf::PdmDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    std::cout << "Client Document File Name: " << clientDocument->fileName() << std::endl;
    ASSERT_EQ( serverApp->document( "testDocument" )->fileName(), clientDocument->fileName() );

    auto clientCapability = clientDocument->capability<caf::rpc::ObjectClientCapability>();
    ASSERT_TRUE( clientCapability != nullptr );
    ASSERT_EQ( reinterpret_cast<uint64_t>( serverDocument ), clientCapability->addressOnServer() );

    std::vector<InheritedDemoObj*> serverDescendants;
    std::vector<InheritedDemoObj*> clientDescendants;

    serverDocument->descendantsOfType( serverDescendants );
    clientDocument->descendantsOfType( clientDescendants );

    ASSERT_EQ( childCount, serverDescendants.size() );
    ASSERT_EQ( serverDescendants.size(), clientDescendants.size() );
    for ( size_t i = 0; i < childCount; ++i )
    {
        auto childClientCapability = clientDescendants[i]->capability<caf::rpc::ObjectClientCapability>();
        ASSERT_TRUE( childClientCapability != nullptr );
        ASSERT_EQ( reinterpret_cast<uint64_t>( serverDescendants[i] ), childClientCapability->addressOnServer() );
    }

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Sync )
{
    int  portNumber = 50000;
    auto serverApp  = std::make_unique<ServerApp>( portNumber );

    ASSERT_TRUE( caf::rpc::ServerApplication::instance() != nullptr );

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

    size_t childCount = 11u;
    for ( size_t i = 0; i < childCount; ++i )
    {
        serverDocument->addInheritedObject( new InheritedDemoObj );
    }

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<caf::PdmDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    std::cout << "Client Document File Name: " << clientDocument->fileName() << std::endl;
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

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

    serverDocument->addInheritedObject( new InheritedDemoObj );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    auto serverObjects = serverDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, serverObjects.size() );
    auto inheritedObjects = clientDocument->inheritedObjects();
    ASSERT_EQ( (size_t)1, inheritedObjects.size() );

    DemoObject_copyObject method( inheritedObjects.front(), 45.3, 43, "AnotherValue" );
    auto                  result = client->execute( &method );
    ASSERT_TRUE( result != nullptr );
    auto copyObjectResult = dynamic_cast<DemoObject_copyObjectResult*>( result.get() );
    ASSERT_TRUE( copyObjectResult && copyObjectResult->status() );

    ASSERT_EQ( 45.3, serverObjects.front()->doubleMember() );
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

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );
    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

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

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

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

    largeDoubleVector = serverDocument->m_demoObject->getDoubleVector();
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

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

    std::vector<double> serverVector;
    std::mt19937        rng;
    std::generate_n( std::back_inserter( serverVector ), 10000u, std::ref( rng ) );

    serverDocument->m_demoObject->setDoubleVector( serverVector );

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );
    auto clientVector = clientDocument->m_demoObject->getDoubleVector();

    ASSERT_EQ( serverVector, clientVector );

    for ( auto& i : clientVector )
    {
        i += 2;
    }
    ASSERT_NE( serverVector, clientVector );
    clientDocument->m_demoObject->setDoubleVector( clientVector );

    serverVector = serverDocument->m_demoObject->getDoubleVector();
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

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );
    caf::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( client.get() );

    auto serverDocument = dynamic_cast<DemoDocument*>( serverApp->document( "testDocument" ) );
    ASSERT_TRUE( serverDocument );
    std::cout << "Server Document File Name: " << serverDocument->fileName() << std::endl;

    auto objectHandle   = client->document( "testDocument" );
    auto clientDocument = dynamic_cast<DemoDocument*>( objectHandle.get() );
    ASSERT_TRUE( clientDocument != nullptr );

    {
        serverDocument->m_demoObject->setFloatVector({42.0f});
        auto start_time = std::chrono::system_clock::now();
        auto clientVector    = clientDocument->m_demoObject->getFloatVector();
        auto end_time   = std::chrono::system_clock::now();
        auto duration   = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count();
        std::cout << "Getting single float vector took " << duration << "µs" << std::endl;        
        ASSERT_EQ(serverDocument->m_demoObject->getFloatVector(), clientDocument->m_demoObject->getFloatVector());
    }

    std::vector<float> serverVector;
    std::mt19937       rng;
    size_t             numberOfFloats = 1024u * 1024u * 128;
    serverVector.reserve( numberOfFloats );
    for ( size_t i = 0; i < numberOfFloats; ++i )
    {
        serverVector.push_back( (float)rng() );
    }

    serverDocument->m_demoObject->setFloatVector( serverVector );

    {
        auto start_time = std::chrono::system_clock::now();
        auto clientVector    = clientDocument->m_demoObject->getFloatVector();
        auto end_time   = std::chrono::system_clock::now();
        auto duration   = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        size_t MB = numberOfFloats * sizeof(float) / (1024u * 1024u);
        std::cout << "Transferred " << numberOfFloats << " floats for a total of " << MB << " MB" << std::endl;        
        std::cout << "Time spent: " << duration << "ms" << std::endl;
        double fps = static_cast<float>( numberOfFloats ) / static_cast<float>( duration ) * 1000;
        std::cout << "floats per second: " << fps << std::endl;
        std::cout << "MB per second: " << static_cast<float>(MB) / static_cast<float>(duration) * 1000 << std::endl;
    }

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}
