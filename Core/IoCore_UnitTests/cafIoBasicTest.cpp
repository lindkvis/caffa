
#include "gtest.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    DemoObject()
    {
        initField( m_proxyDoubleField, "BigNumber" );

        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

        initField( m_proxyEnumField, "EnumField" );
        auto proxyEnumAccessor = std::make_unique<caffa::FieldProxyAccessor<caffa::AppEnum<TestEnumType>>>();
        proxyEnumAccessor->registerSetMethod( this, &DemoObject::setEnumMember );
        proxyEnumAccessor->registerGetMethod( this, &DemoObject::enumMember );
        m_proxyEnumField.setAccessor( std::move( proxyEnumAccessor ) );

        m_enumMember = T1;
    }

    ~DemoObject() {}

    std::string classKeywordDynamic() const override { return "DemoObject"; }

    // Fields

    caffa::Field<caffa::AppEnum<TestEnumType>> m_proxyEnumField;
    caffa::Field<double>                       m_proxyDoubleField;

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

    void setEnumMember( const caffa::AppEnum<TestEnumType>& val ) { m_enumMember = val.value(); }
    caffa::AppEnum<TestEnumType> enumMember() const { return m_enumMember; }

    double       m_doubleMember;
    TestEnumType m_enumMember;
};

CAFFA_SOURCE_INIT( DemoObject, "DemoObject" )

namespace caffa
{
template <>
void AppEnum<DemoObject::TestEnumType>::setUp()
{
    addItem( DemoObject::T1, "T1" );
    addItem( DemoObject::T2, "T2" );
    addItem( DemoObject::T3, "T3" );
    setDefault( DemoObject::T1 );
}

} // namespace caffa

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
        a->m_proxyEnumField = DemoObject::T3;

        ASSERT_DOUBLE_EQ( 2.5, a->m_proxyDoubleField.value() );
        ASSERT_EQ( DemoObject::T3, a->m_proxyEnumField.value() );

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
    CAFFA_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        initField( m_texts, "Texts" );
        initField( m_childArrayField, "DemoObjects" );
    }

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};
CAFFA_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj", "DemoObject" )

class IntRangeValidator : public caffa::FieldValueValidator<int>
{
public:
    IntRangeValidator( int minimum, int maximum )
        : m_minimum( minimum )
        , m_maximum( maximum )
    {
    }

    void readFromJson( const nlohmann::json& jsonValue, const caffa::Serializer& serializer ) override
    {
        std::cout << "Reading range validator from json: " << jsonValue.dump() << std::endl;
        if ( jsonValue.contains( "range" ) )
        {
            std::cout << "Found range!" << jsonValue["range"].dump() << std::endl;
            std::tie( m_minimum, m_maximum ) = jsonValue["range"].get<std::pair<int, int>>();
        }
    }
    void writeToJson( nlohmann::json& jsonValue, const caffa::Serializer& serializer ) const override
    {
        jsonValue["range"] = { m_minimum, m_maximum };
    }
    bool validate( const int& value ) const override { return m_minimum <= value && value <= m_maximum; }

private:
    int m_minimum;
    int m_maximum;
};

class SimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    SimpleObj()
        : m_doubleMember( 0.0 )
    {
        initField( m_position, "Position" );
        initField( m_dir, "Dir" );
        initField( m_up, "Up" );

        initField( m_proxyDouble, "m_proxyDouble" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &SimpleObj::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &SimpleObj::doubleMember );
        m_proxyDouble.setAccessor( std::move( doubleProxyAccessor ) );
    }

    caffa::Field<double> m_position;
    caffa::Field<double> m_dir;
    caffa::Field<int>    m_up;
    caffa::Field<double> m_proxyDouble;

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
    void setUpRange( int minimum, int maximum )
    {
        m_up.setValueValidator( std::make_unique<IntRangeValidator>( minimum, maximum ) );
    }

    std::string classKeywordDynamic() const override { return classKeyword(); }

    double m_doubleMember;
};
CAFFA_SOURCE_INIT( SimpleObj, "SimpleObj" )

class ReferenceDemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    ReferenceDemoObject()
    {
        initField( m_pointersField, "SimpleObjPtrField" );
        initField( m_simpleObjPtrField2, "SimpleObjPtrField2" );
    }

    std::string classKeywordDynamic() const override { return classKeyword(); }

    // Fields
    caffa::ChildField<ObjectHandle*>   m_pointersField;
    caffa::ChildArrayField<SimpleObj*> m_simpleObjPtrField2;
};

CAFFA_SOURCE_INIT( ReferenceDemoObject, "ReferenceDemoObject" )

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

        std::cout << "Wrote object to json: " << serializedString << std::endl;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestRangeValidation )
{
    auto s1 = std::make_unique<SimpleObj>();
    ASSERT_NO_THROW( s1->m_up.setValue( -10 ) );
    ASSERT_NO_THROW( s1->m_up.setValue( 0 ) );
    s1->setUpRange( 0, 10 );
    ASSERT_NO_THROW( s1->m_up.setValue( 5 ) );
    ASSERT_THROW( s1->m_up.setValue( 12 ), std::runtime_error );
    ASSERT_THROW( s1->m_up.setValue( -2 ), std::runtime_error );
    ASSERT_EQ( 5, s1->m_up );

    auto serializedString = caffa::JsonSerializer().writeObjectToString( s1.get() );
    std::cout << "Wrote object to json with validator: " << serializedString << std::endl;

    auto s2 = std::make_unique<SimpleObj>();
    s2->m_up.setValue( 5 );
    auto serializedString2 = caffa::JsonSerializer().writeObjectToString( s2.get() );
    std::cout << "Wrote object to json without validator: " << serializedString2 << std::endl;

    ASSERT_TRUE( serializedString != serializedString2 );

    ASSERT_NO_THROW( s2->m_up.setValue( 12 ) );
    s2->setUpRange( 0, 15 );
    ASSERT_THROW( s2->m_up.setValue( 20 ), std::runtime_error );
    ASSERT_NO_THROW( s2->m_up.setValue( 12 ) );
    caffa::JsonSerializer().readObjectFromString( s2.get(), serializedString );
    ASSERT_THROW( s2->m_up.setValue( 12 ), std::runtime_error );
    auto serializedString3 = caffa::JsonSerializer().writeObjectToString( s2.get() );
    std::cout << "Wrote object to json with validator: " << serializedString3 << std::endl;
    ASSERT_EQ( serializedString, serializedString3 );
    ASSERT_THROW( s2->m_up.setValue( 12 ), std::runtime_error );
}
