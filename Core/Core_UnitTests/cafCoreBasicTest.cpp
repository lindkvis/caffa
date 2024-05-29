
#include "gtest/gtest.h"

#include "Parent.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafMethod.h"
#include "cafObject.h"
#include "cafPortableDataType.h"
#include "cafSession.h"
#include "cafTypedField.h"

#include <functional>
#include <map>
#include <vector>
namespace caffa
{
class Serializer;
}

using IntRangeValidator = caffa::RangeValidator<int>;

using namespace std::placeholders;

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT( DemoObject, Object )

public:
    DemoObject()
    {
        initField( m_proxyDoubleField, "m_proxyDoubleField" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( std::bind( &DemoObject::setDoubleMember, this, _1 ) );
        doubleProxyAccessor->registerGetMethod( std::bind( &DemoObject::doubleMember, this ) );
        m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

        initField( m_proxyIntField, "m_proxyIntField" );
        auto intProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<int>>();
        intProxyAccessor->registerSetMethod( std::bind( &DemoObject::setIntMember, this, _1 ) );
        intProxyAccessor->registerGetMethod( std::bind( &DemoObject::intMember, this ) );
        m_proxyIntField.setAccessor( std::move( intProxyAccessor ) );

        initField( m_proxyStringField, "m_proxyStringField" );
        auto stringProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::string>>();
        stringProxyAccessor->registerSetMethod( std::bind( &DemoObject::setStringMember, this, _1 ) );
        stringProxyAccessor->registerGetMethod( std::bind( &DemoObject::stringMember, this ) );
        m_proxyStringField.setAccessor( std::move( stringProxyAccessor ) );

        initField( m_memberDoubleField, "m_memberDoubleField" );
        initField( m_memberIntField, "m_memberIntField" ).withValidator( IntRangeValidator::create( -10, 1000 ) );
        initField( m_memberStringField, "m_memberStringField" );

        // Default values
        m_doubleMember = 2.1;
        m_intMember    = 7;
        m_stringMember = "abba";

        m_memberDoubleField = 0.0;
        m_memberIntField    = 0;
        m_memberStringField = "";

        initMethod( multiply,
                    "multiply",
                    std::bind( []( int a, int b ) -> double { return a * b; }, std::placeholders::_1, std::placeholders::_2 ) )
            .withArgumentNames( { "a", "b" } )
            .makeConst();

        initMethod( add, "add", std::bind( &DemoObject::_add, this, std::placeholders::_1, std::placeholders::_2 ) )
            .withArgumentNames( { "a", "b" } )
            .makeConst();

        initMethod( clone,
                    "clone",
                    std::bind(
                        [this]() -> std::shared_ptr<DemoObject>
                        {
                            caffa::ObjectFactory* objectFactory = caffa::DefaultObjectFactory::instance();
                            auto                  object =
                                caffa::JsonSerializer( objectFactory ).setSerializeUuids( false ).copyBySerialization( this );
                            return std::dynamic_pointer_cast<DemoObject>( object );
                        } ) )
            .makeConst();

        initMethod( copyFrom, "copyFrom", std::bind( &DemoObject::_copyFrom, this, std::placeholders::_1 ) )
            .withArgumentNames( { "rhs" } )
            .makeConst();
    }

    // Fields
    caffa::Field<double>      m_proxyDoubleField;
    caffa::Field<int>         m_proxyIntField;
    caffa::Field<std::string> m_proxyStringField;

    caffa::Field<double>      m_memberDoubleField;
    caffa::Field<int>         m_memberIntField;
    caffa::Field<std::string> m_memberStringField;

    caffa::Method<double( int, int )>                        multiply;
    caffa::Method<int( int, int )>                           add;
    caffa::Method<std::shared_ptr<DemoObject>()>             clone;
    caffa::Method<void( std::shared_ptr<const DemoObject> )> copyFrom;

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

    int _add( int a, int b ) const { return a + b; }

    void _copyFrom( std::shared_ptr<const DemoObject> rhs )
    {
        caffa::JsonSerializer serializer;
        std::string           json = serializer.writeObjectToString( rhs.get() );
        serializer.readObjectFromString( this, json );
    }

private:
    double      m_doubleMember;
    int         m_intMember;
    std::string m_stringMember;
};

CAFFA_SOURCE_INIT( DemoObject )

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT( InheritedDemoObj, DemoObject )

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

CAFFA_SOURCE_INIT( InheritedDemoObj )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestInheritanceStack )
{
    InheritedDemoObj              demoObj;
    auto                          stack = demoObj.classInheritanceStack();
    std::vector<std::string_view> validEntries;
    for ( auto entry : stack )
    {
        if ( !entry.empty() )
        {
            CAFFA_INFO( "Valid entry in stack for InheritedDemoObj: " << entry );
            validEntries.push_back( entry );
        }
    }
    ASSERT_EQ( (size_t)4, validEntries.size() );
}

//--------------------------------------------------------------------------------------------------
/// TestField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestField )
{
    auto a = std::make_shared<DemoObject>();

    ASSERT_DOUBLE_EQ( 0.0, a->m_memberDoubleField.value() );
    a->m_memberDoubleField.setValue( 1.2 );
    ASSERT_DOUBLE_EQ( 1.2, a->m_memberDoubleField.value() );

    // Simpler access
    ASSERT_DOUBLE_EQ( 1.2, a->m_memberDoubleField );
    a->m_memberDoubleField = 42.0;
    ASSERT_DOUBLE_EQ( 42.0, a->m_memberDoubleField );
    ASSERT_EQ( 0, *a->m_memberIntField );
    ASSERT_NO_THROW( a->m_memberIntField = 1000 );
    ASSERT_NO_THROW( a->m_memberIntField = -10 );
    ASSERT_NO_THROW( a->m_memberIntField = 11 );
    ASSERT_THROW( a->m_memberIntField = 1001, std::runtime_error );
    ASSERT_EQ( 11, a->m_memberIntField.value() );

    ASSERT_TRUE( ( *a->m_memberStringField ).empty() );
    a->m_memberStringField.setValue( "123" );
    ASSERT_TRUE( a->m_memberStringField.value() == "123" );
}

//--------------------------------------------------------------------------------------------------
/// TestProxyTypedField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestProxyTypedField )
{
    auto a = std::make_shared<DemoObject>();

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
    CAFFA_HEADER_INIT( A, Object )

public:
    A() {}

    explicit A( const std::vector<double>& testValue )
    {
        initField( field1, "field1" );
        initField( field2, "field2" );
        initField( field3, "field3" );

        field2 = testValue;
        field3 = field2();
    }

    caffa::Field<std::vector<double>> field1;
    caffa::Field<std::vector<double>> field2;
    caffa::Field<std::vector<double>> field3;
};

CAFFA_SOURCE_INIT( A )

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
    EXPECT_FALSE( a.field1() == a.field3() );
    // = field to field
    a.field1 = a.field2();
    // ()
    EXPECT_EQ( 1.3, a.field1()[2] );
    // = value to field
    a.field1 = testValue2;
    // v()
    EXPECT_EQ( 2.3, a.field1.value()[2] );
    // ==
    a.field3 = a.field1();
    EXPECT_TRUE( a.field1() == a.field3() );
}

//--------------------------------------------------------------------------------------------------
/// Test of ChildArrayField operations
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, ChildArrayField )
{
    auto ihd1 = std::make_shared<InheritedDemoObj>();

    auto s1 = std::make_shared<DemoObject>();
    auto s2 = std::make_shared<DemoObject>();
    auto s3 = std::make_shared<DemoObject>();

    std::weak_ptr<DemoObject> s1p = s1;
    std::weak_ptr<DemoObject> s2p = s2;
    std::weak_ptr<DemoObject> s3p = s3;

    // empty() number 1
    EXPECT_TRUE( ihd1->m_childArrayField.empty() );
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    EXPECT_EQ( 2, s1.use_count() );
    EXPECT_EQ( 2, s2.use_count() );
    EXPECT_EQ( 2, s3.use_count() );

    // size()
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );

    // operator[]
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );
    EXPECT_EQ( s2, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3, ihd1->m_childArrayField[2] );

    // childObjects
    auto objects = ihd1->m_childArrayField.childObjects();
    EXPECT_EQ( size_t( 3 ), objects.size() );
    EXPECT_EQ( 3, s1.use_count() );
    EXPECT_EQ( 3, s2.use_count() );
    EXPECT_EQ( 3, s3.use_count() );
    objects.clear();
    EXPECT_EQ( 2, s1.use_count() );
    EXPECT_EQ( 2, s2.use_count() );
    EXPECT_EQ( 2, s3.use_count() );

    auto typedObjects = ihd1->m_childArrayField.objects();
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );
    EXPECT_EQ( 3, s1.use_count() );
    EXPECT_EQ( 3, s2.use_count() );
    EXPECT_EQ( 3, s3.use_count() );
    typedObjects.clear();

    // remove child object
    ihd1->m_childArrayField.removeChildObject( s2 );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( 2, s1.use_count() );
    EXPECT_EQ( 1, s2.use_count() );
    EXPECT_EQ( 2, s3.use_count() );

    ihd1->m_childArrayField.removeChildObject( nullptr );

    EXPECT_EQ( s3, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );

    // insertAt()
    ihd1->m_childArrayField.insertAt( 1, s2 );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );
    EXPECT_EQ( s2, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3, ihd1->m_childArrayField[2] );

    // erase (index)
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );
    ihd1->m_childArrayField.erase( 1 );
    EXPECT_TRUE( s2 );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( s3, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );

    // clear()
    auto extractedObjects = ihd1->m_childArrayField.objects();

    EXPECT_EQ( 3, s1.use_count() );
    EXPECT_EQ( 1, s2.use_count() );
    EXPECT_EQ( 3, s3.use_count() );

    ihd1->m_childArrayField.clear();

    EXPECT_EQ( size_t( 2 ), extractedObjects.size() );
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    for ( auto& object : extractedObjects )
    {
        ihd1->m_childArrayField.push_back_obj( object );
    }

    s1.reset();
    s2.reset();
    s3.reset();

    EXPECT_TRUE( s1 == nullptr );
    EXPECT_TRUE( s2 == nullptr );
    EXPECT_TRUE( s3 == nullptr );

    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_FALSE( s1p.expired() );
    EXPECT_TRUE( s2p.expired() );
    EXPECT_FALSE( s3p.expired() );

    extractedObjects.clear();
    EXPECT_FALSE( s1p.expired() );
    EXPECT_TRUE( s2p.expired() );
    EXPECT_FALSE( s3p.expired() );

    ihd1->m_childArrayField.clear();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( s1p.expired() );
    EXPECT_TRUE( s2p.expired() );
    EXPECT_TRUE( s3p.expired() );
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
    auto ihd1 = std::make_shared<Parent>();

    auto s1 = std::make_shared<Child>();
    auto s2 = std::make_shared<Child>();
    auto s3 = std::make_shared<Child>();

    std::vector<std::shared_ptr<Child>> typedObjects;
    typedObjects.push_back( s1 );
    typedObjects.push_back( s2 );
    typedObjects.push_back( s3 );

    ihd1->m_simpleObjectsField.push_back( std::make_shared<Child>() );
    for ( auto& typedObject : typedObjects )
    {
        ihd1->m_simpleObjectsField.push_back( typedObject );
    }
    EXPECT_EQ( size_t( 4 ), ihd1->m_simpleObjectsField.size() );
    EXPECT_EQ( ihd1->m_simpleObjectsField[3], s3 );
}

//--------------------------------------------------------------------------------------------------
/// ChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldHandle )
{
    auto s0                 = std::make_shared<DemoObject>();
    s0->m_memberDoubleField = 1000;

    auto s1                 = std::make_shared<DemoObject>();
    s1->m_memberDoubleField = 1000;

    auto s2                 = std::make_shared<DemoObject>();
    s2->m_memberDoubleField = 2000;

    auto s3                 = std::make_shared<DemoObject>();
    s3->m_memberDoubleField = 3000;

    auto                          ihd1      = std::make_shared<InheritedDemoObj>();
    caffa::ChildArrayFieldHandle* listField = &( ihd1->m_childArrayField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->empty() );

    listField->insertAt( 0u, s0 );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

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
    CAFFA_HEADER_INIT( A2, Object )

public:
    explicit A2()
        : b( 0 )
    {
        initField( field2, "field2" );
    }

    caffa::ChildField<Child*> field2;
    int                       b;
};

CAFFA_SOURCE_INIT( A2 )
//--------------------------------------------------------------------------------------------------
/// Test of ChildField
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, ChildField )
{
    {
        std::weak_ptr<Child> weapPtr;

        {
            auto testValue = std::make_shared<Child>();
            weapPtr        = testValue;
            EXPECT_FALSE( weapPtr.expired() );

            A2 a;
            a.field2 = testValue;
            EXPECT_EQ( a.field2, weapPtr.lock() );
        }
        // Guarded
        EXPECT_TRUE( weapPtr.expired() );
    }
    {
        A2   a;
        auto c2 = std::make_shared<Child>();
        // Assign
        a.field2.setObject( c2 );
        // Access
        EXPECT_EQ( c2, a.field2 );
        EXPECT_EQ( c2, a.field2.object() );
        EXPECT_TRUE( c2 == a.field2 );

        std::vector<caffa::ObjectHandle::Ptr> objects = a.field2.childObjects();
        EXPECT_EQ( (size_t)1, objects.size() );
        EXPECT_EQ( c2, objects[0] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of Pointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Pointer )
{
    auto d = std::make_shared<InheritedDemoObj>();

    {
        std::weak_ptr<InheritedDemoObj> p;
        EXPECT_TRUE( p.expired() );
    }

    {
        std::weak_ptr<InheritedDemoObj> p  = d;
        std::weak_ptr<InheritedDemoObj> p2 = p.lock();

        EXPECT_EQ( p.lock().get(), d.get() );
        EXPECT_EQ( p2.lock().get(), d.get() );
        p.reset();
        EXPECT_TRUE( p.lock() == nullptr );
        EXPECT_TRUE( p.expired() );
        p = p2.lock();
        EXPECT_TRUE( p.lock() == d );
        d.reset();
        EXPECT_TRUE( p.expired() && p2.expired() );
    }
}

class ObjectWithPointerInField : public caffa::Object
{
    CAFFA_HEADER_INIT( DemoObject, Object )

public:
    ObjectWithPointerInField() { initField( fieldWithPointer, "fieldWithPointer" ); }

    caffa::Field<std::shared_ptr<DemoObject>> fieldWithPointer;
};

TEST( BaseTest, PointerInRegularField )
{
    ObjectWithPointerInField object;
    object.fieldWithPointer = std::make_shared<DemoObject>();

    // object.fieldWithPointer->
}

TEST( BaseTest, Methods )
{
    DemoObject object;

    EXPECT_EQ( 5, object.add( 3, 2 ) );
    EXPECT_DOUBLE_EQ( 12.0, object.multiply( 4, 3 ) );

    object.setIntMember( 513 );
    object.setDoubleMember( 589.123 );

    EXPECT_EQ( 513, object.intMember() );
    EXPECT_DOUBLE_EQ( 589.123, object.doubleMember() );

    CAFFA_DEBUG( "Clone method skeleton: " << object.clone.jsonSchema().dump() );

    auto result = object.clone();
    EXPECT_TRUE( result != nullptr );
    EXPECT_EQ( object.intMember(), result->intMember() );
    EXPECT_DOUBLE_EQ( object.doubleMember(), result->doubleMember() );

    caffa::JsonSerializer serializer;
    CAFFA_DEBUG( "Clone: " << serializer.writeObjectToString( result.get() ) );

    auto session = caffa::Session::create( caffa::Session::Type::REGULAR );

    {
        CAFFA_DEBUG( "Method skeleton: " << object.add.jsonSchema().dump() );

        auto argumentJson = object.add.toJson( 3, 8 );
        CAFFA_DEBUG( "Argument json: " << argumentJson.dump() );
        auto stringResult = object.add.execute( session, argumentJson.dump() );
        CAFFA_DEBUG( "String result: " << stringResult );
        auto result = nlohmann::json::parse( stringResult );
        EXPECT_EQ( 11, result.get<int>() );
    }

    {
        auto result = nlohmann::json::parse( object.multiply.execute( session, object.multiply.toJson( 4, 5 ).dump() ) );
        EXPECT_DOUBLE_EQ( 20.0, result.get<int>() );
    }

    {
        auto stringArguments = object.multiply.toJson( 4, 5 );
        CAFFA_DEBUG( "Parameter json: " << stringArguments );
    }

    result->setIntMember( 10001 );
    EXPECT_EQ( 10001, result->intMember() );
    object.copyFrom( result );
    EXPECT_EQ( 10001, object.intMember() );

    CAFFA_DEBUG( "Method skeleton for copyFrom(): " << object.copyFrom.jsonSchema().dump() );
}
