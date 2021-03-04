
#include "gtest/gtest.h"

#include "Parent.h"

#include "../cafGrpcClient.h"
#include "../cafGrpcObjectClientCapability.h"
#include "../cafGrpcServer.h"
#include "../cafGrpcServerApplication.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafObjectHandle.h"
#include "cafObjectScriptingCapability.h"
#include "cafPdmDocument.h"
#include "cafPdmReferenceHelper.h"
#include "cafProxyValueField.h"
#include "cafPtrField.h"
#include "cafValueField.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject()
    {
        this->addField( &m_proxyDoubleField, "m_proxyDoubleField" );
        m_proxyDoubleField.registerSetMethod( this, &DemoObject::setDoubleMember );
        m_proxyDoubleField.registerGetMethod( this, &DemoObject::doubleMember );

        this->addField( &m_proxyIntField, "m_proxyIntField" );
        m_proxyIntField.registerSetMethod( this, &DemoObject::setIntMember );
        m_proxyIntField.registerGetMethod( this, &DemoObject::intMember );

        this->addField( &m_proxyStringField, "m_proxyStringField" );
        m_proxyStringField.registerSetMethod( this, &DemoObject::setStringMember );
        m_proxyStringField.registerGetMethod( this, &DemoObject::stringMember );

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

    // Internal class members accessed by proxy fields
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }
    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }

    int  intMember() const { return m_intMember; }
    void setIntMember( const int& val ) { m_intMember = val; }

    std::string stringMember() const { return m_stringMember; }
    void        setStringMember( const std::string& val ) { m_stringMember = val; }

private:
    double      m_doubleMember;
    int         m_intMember;
    std::string m_stringMember;
};

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

class InheritedDemoObj : public DemoObject
{
    CAF_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        this->addField( &m_texts, "Texts" );
        this->addField( &m_childArrayField, "DemoObjectects" );
        this->addField( &m_ptrField, "m_ptrField" );

        this->addField( &m_singleFilePath, "m_singleFilePath" );
        this->addField( &m_multipleFilePath, "m_multipleFilePath" );
    }

    caf::Field<std::string>           m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
    caf::PtrField<InheritedDemoObj*>  m_ptrField;

    caf::Field<std::filesystem::path>              m_singleFilePath;
    caf::Field<std::vector<std::filesystem::path>> m_multipleFilePath;
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

    caf::ChildField<DemoObject*>            m_demoObject;
    caf::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

CAF_SOURCE_INIT( DemoDocument, "DemoDocument" );

class ServerApp : public caf::GrpcServerApplication
{
public:
    ServerApp( int port )
        : caf::GrpcServerApplication( port )
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

    ASSERT_TRUE( caf::GrpcServerApplication::instance() != nullptr );

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );

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

    ASSERT_TRUE( caf::GrpcServerApplication::instance() != nullptr );

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );

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

    ASSERT_TRUE( caf::GrpcServerApplication::instance() != nullptr );

    std::cout << "Launching Server" << std::endl;
    auto thread = std::thread( &ServerApp::run, serverApp.get() );

    std::cout << "Launching Client" << std::endl;
    while ( !serverApp->running() )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }
    auto client = std::make_unique<caf::rpc::Client>( "localhost", portNumber );

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
    client->sync( clientDocument );
    ASSERT_EQ( newFileName, clientDocument->fileName() );
    ASSERT_EQ( newFileName, serverDocument->fileName() );

    bool ok = client->stopServer();
    ASSERT_TRUE( ok );

    thread.join();
}
