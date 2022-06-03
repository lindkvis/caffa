
#include "gtest.h"

#include "Parent.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObject.h"
#include "cafPortableDataType.h"
#include "cafTypedField.h"

#include <map>
#include <vector>
namespace caffa
{
class Serializer;
}

using IntRangeValidator = caffa::RangeValidator<int>;

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    DemoObject()
    {
        initField( m_proxyDoubleField, "m_proxyDoubleField" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

        initField( m_proxyIntField, "m_proxyIntField" );
        auto intProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<int>>();
        intProxyAccessor->registerSetMethod( this, &DemoObject::setIntMember );
        intProxyAccessor->registerGetMethod( this, &DemoObject::intMember );
        m_proxyIntField.setAccessor( std::move( intProxyAccessor ) );

        initField( m_proxyStringField, "m_proxyStringField" );
        auto stringProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::string>>();
        stringProxyAccessor->registerSetMethod( this, &DemoObject::setStringMember );
        stringProxyAccessor->registerGetMethod( this, &DemoObject::stringMember );
        m_proxyStringField.setAccessor( std::move( stringProxyAccessor ) );

        initField( m_memberDoubleField, "m_memberDoubleField" );
        initField( m_memberIntField, "m_memberIntField" );
        initField( m_memberStringField, "m_memberStringField" );

        m_memberIntField.addValidator( std::make_unique<IntRangeValidator>( -10, 1000 ) );

        // Default values
        m_doubleMember = 2.1;
        m_intMember    = 7;
        m_stringMember = "abba";

        m_memberDoubleField = 0.0;
        m_memberIntField    = 0;
        m_memberStringField = "";
    }

    // Fields
    caffa::Field<double>      m_proxyDoubleField;
    caffa::Field<int>         m_proxyIntField;
    caffa::Field<std::string> m_proxyStringField;

    caffa::Field<double>      m_memberDoubleField;
    caffa::Field<int>         m_memberIntField;
    caffa::Field<std::string> m_memberStringField;

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

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" )

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        initField( m_texts, "Texts" );
        initField( m_childArrayField, "DemoObjectects" );
    }

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};

TEST( BaseTest, Delete )
{
    DemoObject* s2 = new DemoObject;
    delete s2;
}

CAFFA_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj", "DemoObject", "Object" )

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestField )
{
    auto a = std::make_unique<DemoObject>();

    ASSERT_DOUBLE_EQ( 0.0, a->m_memberDoubleField.value() );
    a->m_memberDoubleField.setValue( 1.2 );
    ASSERT_DOUBLE_EQ( 1.2, a->m_memberDoubleField.value() );

    ASSERT_EQ( 0, a->m_memberIntField.value() );
    ASSERT_NO_THROW( a->m_memberIntField.setValue( 1000 ) );
    ASSERT_NO_THROW( a->m_memberIntField.setValue( -10 ) );
    ASSERT_NO_THROW( a->m_memberIntField.setValue( 11 ) );
    ASSERT_THROW( a->m_memberIntField.setValue( 1001 ), std::runtime_error );
    ASSERT_EQ( 11, a->m_memberIntField.value() );

    ASSERT_TRUE( a->m_memberStringField.value().empty() );
    a->m_memberStringField.setValue( "123" );
    ASSERT_TRUE( a->m_memberStringField.value() == "123" );
}

//--------------------------------------------------------------------------------------------------
/// TestProxyTypedField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestProxyTypedField )
{
    auto a = std::make_unique<DemoObject>();

    ASSERT_DOUBLE_EQ( 2.1, a->m_proxyDoubleField.value() );
    a->m_proxyDoubleField.setValue( 1.2 );
    ASSERT_DOUBLE_EQ( 1.2, a->m_proxyDoubleField.value() );

    ASSERT_EQ( 7, a->m_proxyIntField.value() );
    a->m_proxyIntField.setValue( 11 );
    ASSERT_EQ( 11, a->m_proxyIntField.value() );

    ASSERT_TRUE( a->m_proxyStringField.value() == "abba" );
    a->m_proxyStringField.setValue( "123" );
    ASSERT_TRUE( a->m_proxyStringField.value() == "123" );
}

class A : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    A() {}

    explicit A( const std::vector<double>& testValue )
        : field2( testValue )
        , field3( field2 )
    {
        initField( field1, "field1" );
        initField( field2, "field2" );
        initField( field3, "field3" );
    }

    caffa::Field<std::vector<double>> field1;
    caffa::Field<std::vector<double>> field2;
    caffa::Field<std::vector<double>> field3;
};

CAFFA_SOURCE_INIT( A, "A", "Object" )

//--------------------------------------------------------------------------------------------------
/// Test of Field operations
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, NormalField )
{
    std::vector<double> testValue;
    testValue.push_back( 1.1 );
    testValue.push_back( 1.2 );
    testValue.push_back( 1.3 );

    std::vector<double> testValue2;
    testValue2.push_back( 2.1 );
    testValue2.push_back( 2.2 );
    testValue2.push_back( 2.3 );

    // Constructors

    A a( testValue );

    EXPECT_EQ( 1.3, a.field2.value()[2] );
    EXPECT_EQ( 1.3, a.field3.value()[2] );
    EXPECT_EQ( size_t( 0 ), a.field1().size() );

    // Operators
    // ==
    EXPECT_FALSE( a.field1 == a.field3 );
    // = field to field
    a.field1 = a.field2;
    // ()
    EXPECT_EQ( 1.3, a.field1()[2] );
    // = value to field
    a.field1 = testValue2;
    // v()
    EXPECT_EQ( 2.3, a.field1.value()[2] );
    // ==
    a.field3 = a.field1;
    EXPECT_TRUE( a.field1 == a.field3 );
}

//--------------------------------------------------------------------------------------------------
/// Test of ChildArrayField operations
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, ChildArrayField )
{
    auto ihd1 = std::make_unique<InheritedDemoObj>();

    auto s1 = std::make_unique<DemoObject>();
    auto s2 = std::make_unique<DemoObject>();
    auto s3 = std::make_unique<DemoObject>();

    // empty() number 1
    EXPECT_TRUE( ihd1->m_childArrayField.empty() );
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    // push_back()
    auto s1p = caffa::ObservingPointer<DemoObject>( s1.get() );
    auto s2p = caffa::ObservingPointer<DemoObject>( s2.get() );
    auto s3p = caffa::ObservingPointer<DemoObject>( s3.get() );

    ihd1->m_childArrayField.push_back( std::move( s1 ) );
    ihd1->m_childArrayField.push_back( std::move( s2 ) );
    ihd1->m_childArrayField.push_back( std::move( s3 ) );

    // Parent field
    EXPECT_EQ( s1p->parentField(), &( ihd1->m_childArrayField ) );

    // size()
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );

    // operator[]
    EXPECT_EQ( s2p, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3p, ihd1->m_childArrayField[2] );

    // childObjects
    std::vector<caffa::ObjectHandle*> objects = ihd1->m_childArrayField.childObjects();
    EXPECT_EQ( size_t( 3 ), objects.size() );

    std::vector<DemoObject*> typedObjects = ihd1->m_childArrayField.value();
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );

    // remove child object
    auto new_s2 = ihd1->m_childArrayField.removeChildObject( s2p );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( new_s2->parentField() == nullptr );

    auto emptyPointer = ihd1->m_childArrayField.removeChildObject( nullptr );
    EXPECT_TRUE( !emptyPointer );
    EXPECT_EQ( s3p, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1p, ihd1->m_childArrayField[0] );

    EXPECT_EQ( s2p, new_s2.get() );
    // insertAt()
    ihd1->m_childArrayField.insertAt( 1, std::move( new_s2 ) );
    EXPECT_EQ( s1p, ihd1->m_childArrayField[0] );
    EXPECT_EQ( s2p, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3p, ihd1->m_childArrayField[2] );

    EXPECT_TRUE( s2p->parentField() == &( ihd1->m_childArrayField ) );

    // erase (index)
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );
    ihd1->m_childArrayField.erase( 1 );
    EXPECT_TRUE( s2p.isNull() );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( s3p, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1p, ihd1->m_childArrayField[0] );

    // clear()
    auto extractedObjects = ihd1->m_childArrayField.clear();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( size_t( 2 ), extractedObjects.size() );

    EXPECT_TRUE( s1p->parentField() == nullptr );

    for ( auto& object : extractedObjects )
    {
        ihd1->m_childArrayField.push_back_obj( std::move( object ) );
    }
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( s1p.notNull() );
    EXPECT_TRUE( s2p.isNull() );
    EXPECT_TRUE( s3p.notNull() );

    ihd1->m_childArrayField.clear();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( s1p.isNull() );
    EXPECT_TRUE( s2p.isNull() );
    EXPECT_TRUE( s3p.isNull() );
}

TEST( BaseTest, ChildArrayParentField )
{
    // Test of instanciating a class with forward declare of object used in ChildArrayField and ChildField
    Parent* parentObj = new Parent;

    delete parentObj;
}

#include "Child.h"

TEST( BaseTest, PointersFieldInsertVector )
{
    auto ihd1 = std::make_unique<Parent>();

    auto s1 = std::make_unique<Child>();
    auto s2 = std::make_unique<Child>();
    auto s3 = std::make_unique<Child>();

    std::vector<std::unique_ptr<Child>> typedObjects;
    typedObjects.push_back( std::move( s1 ) );
    typedObjects.push_back( std::move( s2 ) );
    auto s3p = s3.get();
    typedObjects.push_back( std::move( s3 ) );

    ihd1->m_simpleObjectsField.push_back( std::make_unique<Child>() );
    for ( auto& typedObject : typedObjects )
    {
        ihd1->m_simpleObjectsField.push_back( std::move( typedObject ) );
    }
    EXPECT_EQ( size_t( 4 ), ihd1->m_simpleObjectsField.size() );
    EXPECT_EQ( ihd1->m_simpleObjectsField[3], s3p );
}

//--------------------------------------------------------------------------------------------------
/// ChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldHandle )
{
    auto s0                 = std::make_unique<DemoObject>();
    s0->m_memberDoubleField = 1000;

    auto s1                 = std::make_unique<DemoObject>();
    s1->m_memberDoubleField = 1000;

    auto s2                 = std::make_unique<DemoObject>();
    s2->m_memberDoubleField = 2000;

    auto s3                 = std::make_unique<DemoObject>();
    s3->m_memberDoubleField = 3000;

    auto                          ihd1      = std::make_unique<InheritedDemoObj>();
    caffa::ChildArrayFieldHandle* listField = &( ihd1->m_childArrayField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->empty() );

    listField->insertAt( 0u, std::move( s0 ) );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_childArrayField.push_back( std::move( s1 ) );
    ihd1->m_childArrayField.push_back( std::move( s2 ) );
    ihd1->m_childArrayField.push_back( std::move( s3 ) );

    EXPECT_EQ( 4u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    listField->erase( 0 );
    EXPECT_EQ( 3u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    listField->clear();
    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->empty() );
}

class A2 : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    explicit A2()
        : b( 0 )
    {
        initField( field2, "field2" );
    }

    caffa::ChildField<Child*> field2;
    int                       b;
};

CAFFA_SOURCE_INIT( A2, "A2", "Object" )
//--------------------------------------------------------------------------------------------------
/// Test of ChildField
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, ChildField )
{
    {
        auto                           testValue = std::make_unique<Child>();
        caffa::ObservingPointer<Child> rawValue  = testValue.get();

        {
            A2 a;
            a.field2 = std::move( testValue );
            EXPECT_EQ( rawValue.p(), a.field2() );
        }
        // Guarded
        EXPECT_TRUE( rawValue.isNull() );
    }
    {
        A2   a;
        auto c2   = std::make_unique<Child>();
        auto rawC = c2.get();
        // Assign
        a.field2.setValue( std::move( c2 ) );
        // Access
        EXPECT_EQ( rawC, a.field2 );
        EXPECT_EQ( rawC, a.field2.value() );
        EXPECT_TRUE( rawC == a.field2 );

        std::vector<caffa::ObjectHandle*> objects = a.field2.childObjects();
        EXPECT_EQ( (size_t)1, objects.size() );
        EXPECT_EQ( rawC, objects[0] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of Pointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Pointer )
{
    auto d = std::make_unique<InheritedDemoObj>();

    {
        caffa::ObservingPointer<InheritedDemoObj> p;
        EXPECT_TRUE( p == nullptr );
    }

    {
        caffa::ObservingPointer<InheritedDemoObj> p( d.get() );
        caffa::ObservingPointer<InheritedDemoObj> p2( p );

        EXPECT_EQ( p, d.get() );
        EXPECT_EQ( p2, d.get() );
        EXPECT_TRUE( p.p() == d.get() );
        p = 0;
        EXPECT_TRUE( p == nullptr );
        EXPECT_TRUE( p.isNull() );
        EXPECT_TRUE( p2 == d.get() );
        p = p2;
        EXPECT_TRUE( p == d.get() );
        d.reset();
        EXPECT_TRUE( p.isNull() && p2.isNull() );
    }

    caffa::ObservingPointer<DemoObject> p3( new DemoObject() );

    delete p3;
}

TEST( BaseTest, PortableDataType )
{
    EXPECT_EQ( "int32", caffa::PortableDataType<int>::name() );
    EXPECT_EQ( "uint32", caffa::PortableDataType<unsigned>::name() );
    EXPECT_EQ( "string", caffa::PortableDataType<std::string>::name() );
    EXPECT_EQ( "double", caffa::PortableDataType<double>::name() );
    EXPECT_EQ( "float", caffa::PortableDataType<float>::name() );
    EXPECT_EQ( "char", caffa::PortableDataType<char>::name() );
    EXPECT_EQ( "bool", caffa::PortableDataType<bool>::name() );
    EXPECT_EQ( "int32[]", caffa::PortableDataType<std::vector<int>>::name() );
    EXPECT_EQ( "uint32[]", caffa::PortableDataType<std::vector<uint32_t>>::name() );
    EXPECT_EQ( "string[]", caffa::PortableDataType<std::vector<std::string>>::name() );
}
