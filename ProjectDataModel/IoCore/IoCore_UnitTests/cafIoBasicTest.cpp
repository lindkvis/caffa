
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapability.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"

class DemoObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    DemoObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_proxyDoubleField, "BigNumber" );

        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );
    }

    ~DemoObject() {}

    std::string classKeywordDynamic() const override { return "DemoObject"; }

    // Fields

    caffa::DataValueField<double> m_proxyDoubleField;

private:
    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }

    double m_doubleMember;
};

CAFFA_IO_SOURCE_INIT( DemoObject, "DemoObject" );

TEST( BaseTest, Delete )
{
    DemoObject* s2 = new DemoObject;
    delete s2;
}

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, FieldWrite )
{
    std::string serializedString;
    {
        auto a = std::make_unique<DemoObject>();

        a->m_proxyDoubleField.setValue( 2.5 );
        ASSERT_DOUBLE_EQ( 2.5, a->m_proxyDoubleField.value() );

        serializedString = caffa::JsonSerializer().writeObjectToString( a.get() );

        std::cout << serializedString << std::endl;
    }

    std::string secondSerializedString;
    {
        auto a                 = caffa::JsonSerializer().createObjectFromString( serializedString );
        secondSerializedString = caffa::JsonSerializer().writeObjectToString( a.get() );
    }
    ASSERT_EQ( serializedString, secondSerializedString );
}

class InheritedDemoObj : public DemoObject
{
    CAFFA_IO_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        CAFFA_IO_InitField( &m_texts, "Texts" );
        CAFFA_IO_InitField( &m_childArrayField, "DemoObjectects" );
    }

    caffa::DataValueField<std::string>  m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};
CAFFA_IO_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj", "DemoObject" );

class SimpleObj : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    SimpleObj()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
        , m_doubleMember( 0.0 )
    {
        CAFFA_IO_InitField( &m_position, "Position" );
        CAFFA_IO_InitField( &m_dir, "Dir" );
        CAFFA_IO_InitField( &m_up, "Up" );

        CAFFA_IO_InitField( &m_proxyDouble, "m_proxyDouble" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &SimpleObj::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &SimpleObj::doubleMember );
        m_proxyDouble.setAccessor( std::move( doubleProxyAccessor ) );
    }

    caffa::DataValueField<double> m_position;
    caffa::DataValueField<double> m_dir;
    caffa::DataValueField<int>    m_up;
    caffa::DataValueField<double> m_proxyDouble;

    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }
    std::string classKeywordDynamic() const override { return classKeyword(); }

    double m_doubleMember;
};
CAFFA_IO_SOURCE_INIT( SimpleObj, "SimpleObj" );

class ReferenceDemoObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    ReferenceDemoObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_pointersField, "SimpleObjPtrField" );
        CAFFA_IO_InitField( &m_simpleObjPtrField2, "SimpleObjPtrField2" );
    }

    std::string classKeywordDynamic() const override { return classKeyword(); }

    // Fields
    caffa::ChildField<ObjectHandle*>   m_pointersField;
    caffa::ChildArrayField<SimpleObj*> m_simpleObjPtrField2;
};

CAFFA_IO_SOURCE_INIT( ReferenceDemoObject, "ReferenceDemoObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldSerializing )
{
    auto s1 = std::make_unique<DemoObject>();
    auto s2 = std::make_unique<DemoObject>();
    auto s3 = std::make_unique<DemoObject>();

    s1->m_proxyDoubleField.setValue( 10 );
    s2->m_proxyDoubleField.setValue( 20 );
    s3->m_proxyDoubleField.setValue( 30 );

    std::string serializedString;
    {
        auto ihd1 = std::make_unique<InheritedDemoObj>();
        auto s1p  = s1.get();
        ihd1->m_childArrayField.push_back( std::move( s1 ) );
        auto s2p = s2.get();
        ihd1->m_childArrayField.push_back( std::move( s2 ) );
        auto s3p = s3.get();
        ihd1->m_childArrayField.push_back( std::move( s3 ) );

        serializedString = caffa::JsonSerializer().writeObjectToString( ihd1.get() );

        std::cout << "Write object to json: " << serializedString << std::endl;
    }

    {
        auto ihd1 = std::make_unique<InheritedDemoObj>();
        ASSERT_EQ( 0u, ihd1->m_childArrayField.size() );

        caffa::JsonSerializer().readObjectFromString( ihd1.get(), serializedString );
        ASSERT_EQ( 3u, ihd1->m_childArrayField.size() );

        ASSERT_TRUE( ihd1->m_childArrayField[0] != nullptr );
        ASSERT_TRUE( ihd1->m_childArrayField[1] != nullptr );
        ASSERT_TRUE( ihd1->m_childArrayField[2] != nullptr );
        ASSERT_DOUBLE_EQ( 10, ihd1->m_childArrayField[0]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 20, ihd1->m_childArrayField[1]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 30, ihd1->m_childArrayField[2]->m_proxyDoubleField.value() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestDataType )
{
    auto s1 = std::make_unique<SimpleObj>();

    {
        auto dataType = s1->m_position.dataType();
        EXPECT_EQ( "double", dataType );
    }

    {
        auto dataType = s1->m_proxyDouble.dataType();
        EXPECT_EQ( "double", dataType );
    }

    {
        auto dataType = s1->m_up.dataType();
        EXPECT_EQ( "int32", dataType );
    }

    {
        auto obj      = std::make_unique<InheritedDemoObj>();
        auto dataType = obj->m_childArrayField.dataType();
        EXPECT_EQ( std::string( "object[]" ), dataType );
    }
}
