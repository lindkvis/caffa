
#include "gtest/gtest.h"

#include "Parent.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPtrField.h"
#include "cafValueField.h"

#include <vector>

class DemoObject : public caf::ObjectHandle
{
public:
    DemoObject()
    {
        this->addField( &m_proxyDoubleField, "m_proxyDoubleField" );
        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setFieldDataAccessor( std::move( doubleProxyAccessor ) );

        this->addField( &m_proxyIntField, "m_proxyIntField" );
        auto intProxyAccessor = std::make_unique<caf::FieldProxyAccessor<int>>();
        intProxyAccessor->registerSetMethod( this, &DemoObject::setIntMember );
        intProxyAccessor->registerGetMethod( this, &DemoObject::intMember );
        m_proxyIntField.setFieldDataAccessor( std::move( intProxyAccessor ) );

        this->addField( &m_proxyStringField, "m_proxyStringField" );
        auto stringProxyAccessor = std::make_unique<caf::FieldProxyAccessor<std::string>>();
        stringProxyAccessor->registerSetMethod( this, &DemoObject::setStringMember );
        stringProxyAccessor->registerGetMethod( this, &DemoObject::stringMember );
        m_proxyStringField.setFieldDataAccessor( std::move( stringProxyAccessor ) );

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

    // Fields
    caf::DataValueField<double>      m_proxyDoubleField;
    caf::DataValueField<int>         m_proxyIntField;
    caf::DataValueField<std::string> m_proxyStringField;

    caf::DataValueField<double>      m_memberDoubleField;
    caf::DataValueField<int>         m_memberIntField;
    caf::DataValueField<std::string> m_memberStringField;

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

class InheritedDemoObj : public DemoObject
{
public:
    InheritedDemoObj()
    {
        this->addField( &m_texts, "Texts" );
        this->addField( &m_childArrayField, "DemoObjectects" );
        this->addField( &m_ptrField, "m_ptrField" );
    }

    caf::DataValueField<std::string>  m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
    caf::PtrField<InheritedDemoObj*>  m_ptrField;
};

TEST( BaseTest, Delete )
{
    DemoObject* s2 = new DemoObject;
    delete s2;
}

//--------------------------------------------------------------------------------------------------
/// TestDataValueField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestDataValueField )
{
    auto a = std::make_unique<DemoObject>();

    ASSERT_DOUBLE_EQ( 0.0, a->m_memberDoubleField.value() );
    a->m_memberDoubleField.setValue( 1.2 );
    ASSERT_DOUBLE_EQ( 1.2, a->m_memberDoubleField.value() );

    ASSERT_EQ( 0, a->m_memberIntField.value() );
    a->m_memberIntField.setValue( 11 );
    ASSERT_EQ( 11, a->m_memberIntField.value() );

    ASSERT_TRUE( a->m_memberStringField.value().empty() );
    a->m_memberStringField.setValue( "123" );
    ASSERT_TRUE( a->m_memberStringField.value() == "123" );
}

//--------------------------------------------------------------------------------------------------
/// TestProxyValueField
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestProxyValueField )
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

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestValueFieldInterface )
{
    auto a = std::make_unique<DemoObject>();

    {
        caf::ValueField* valField    = dynamic_cast<caf::ValueField*>( a->findField( "m_proxyDoubleField" ) );
        caf::Variant     originalVal = 3.4;
        valField->setFromVariant( originalVal );
        caf::Variant newVal = valField->toVariant();
        ASSERT_EQ( originalVal.value<double>(), newVal.value<double>() );
    }

    {
        caf::ValueField* valField    = dynamic_cast<caf::ValueField*>( a->findField( "m_proxyIntField" ) );
        caf::Variant     originalVal = 3;
        valField->setFromVariant( originalVal );
        caf::Variant newVal = valField->toVariant();
        ASSERT_EQ( originalVal.value<int>(), newVal.value<int>() );
    }

    {
        caf::ValueField* valField    = dynamic_cast<caf::ValueField*>( a->findField( "m_proxyStringField" ) );
        caf::Variant     originalVal = "test";
        valField->setFromVariant( originalVal );
        caf::Variant newVal = valField->toVariant();
        ASSERT_EQ( originalVal.value<std::string>(), newVal.value<std::string>() );
    }

    {
        caf::ValueField* valField    = dynamic_cast<caf::ValueField*>( a->findField( "m_memberDoubleField" ) );
        caf::Variant     originalVal = 3.4;
        valField->setFromVariant( originalVal );
        caf::Variant newVal = valField->toVariant();
        ASSERT_EQ( originalVal.value<double>(), newVal.value<double>() );
    }

    {
        caf::ValueField* valField    = dynamic_cast<caf::ValueField*>( a->findField( "m_memberIntField" ) );
        caf::Variant     originalVal = 3;
        valField->setFromVariant( originalVal );
        caf::Variant newVal = valField->toVariant();
        ASSERT_EQ( originalVal.value<int>(), newVal.value<int>() );
    }

    {
        caf::ValueField* valField    = dynamic_cast<caf::ValueField*>( a->findField( "m_memberStringField" ) );
        caf::Variant     originalVal = "test";
        valField->setFromVariant( originalVal );
        caf::Variant newVal = valField->toVariant();
        ASSERT_EQ( originalVal.value<std::string>(), newVal.value<std::string>() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test of DataValueField operations
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, NormalField )
{
    class A : public caf::ObjectHandle
    {
    public:
        explicit A( const std::vector<double>& testValue )
            : field2( testValue )
            , field3( field2 )
        {
            this->addField( &field1, "field1" );
            this->addField( &field2, "field2" );
            this->addField( &field3, "field3" );
        }

        caf::DataValueField<std::vector<double>> field1;
        caf::DataValueField<std::vector<double>> field2;
        caf::DataValueField<std::vector<double>> field3;
    };

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
    auto s1p = ihd1->m_childArrayField.push_back( std::move( s1 ) );
    auto s2p = ihd1->m_childArrayField.push_back( std::move( s2 ) );
    auto s3p = ihd1->m_childArrayField.push_back( std::move( s3 ) );

    // Parent field
    EXPECT_EQ( s1p->parentField(), &( ihd1->m_childArrayField ) );

    // size()
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );

    // operator[]
    EXPECT_EQ( s2p, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3p, ihd1->m_childArrayField[2] );

    // childObjects
    std::vector<caf::ObjectHandle*> objects;
    ihd1->m_childArrayField.childObjects( &objects );
    EXPECT_EQ( size_t( 3 ), objects.size() );

    std::vector<DemoObject*> typedObjects = ihd1->m_childArrayField.childObjects();
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );

    // remove child object
    auto new_s2 = ihd1->m_childArrayField.remove( s2p );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( new_s2->parentField() == nullptr );

    auto emptyPointer = ihd1->m_childArrayField.remove( nullptr );
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
    auto extractedObjects = ihd1->m_childArrayField.removeAll();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( size_t( 2 ), extractedObjects.size() );

    EXPECT_TRUE( s1p->parentField() == nullptr );

    for ( auto& object : extractedObjects )
    {
        ihd1->m_childArrayField.push_back( std::move( object ) );
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

    auto                        ihd1      = std::make_unique<InheritedDemoObj>();
    caf::ChildArrayFieldHandle* listField = &( ihd1->m_childArrayField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );

    listField->insertAt( 0u, std::move( s0 ) );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_childArrayField.push_back( std::move( s1 ) );
    ihd1->m_childArrayField.push_back( std::move( s2 ) );
    ihd1->m_childArrayField.push_back( std::move( s3 ) );

    EXPECT_EQ( 4u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->erase( 0 );
    EXPECT_EQ( 3u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->clear();
    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );
}
//--------------------------------------------------------------------------------------------------
/// Test of ChildField
//--------------------------------------------------------------------------------------------------

TEST( BaseTest, ChildField )
{
    class A : public caf::ObjectHandle
    {
    public:
        explicit A()
            : b( 0 )
        {
            this->addField( &field2, "field2" );
        }

        caf::ChildField<Child*> field2;
        int                     b;
    };

    {
        auto                testValue = std::make_unique<Child>();
        caf::Pointer<Child> rawValue  = testValue.get();

        {
            A a;
            a.field2 = std::move( testValue );
            EXPECT_EQ( rawValue.p(), a.field2() );
        }
        // Guarded
        EXPECT_TRUE( rawValue.isNull() );
    }
    {
        A    a;
        auto c2   = std::make_unique<Child>();
        auto rawC = c2.get();
        // Assign
        a.field2.setValue( std::move( c2 ) );
        // Access
        EXPECT_EQ( rawC, a.field2 );
        EXPECT_EQ( rawC, a.field2.value() );
        EXPECT_TRUE( rawC == a.field2 );

        std::vector<caf::ObjectHandle*> objects;
        a.field2.childObjects( &objects );
        EXPECT_EQ( (size_t)1, objects.size() );
        EXPECT_EQ( rawC, objects[0] );
    }
}

TEST( BaseTest, PtrField )
{
    auto ihd1 = std::make_unique<InheritedDemoObj>();
    auto ihd2 = std::make_unique<InheritedDemoObj>();

    // Direct access
    EXPECT_EQ( static_cast<InheritedDemoObj*>( nullptr ), ihd1->m_ptrField );

    // Assignment
    ihd1->m_ptrField              = ihd1.get();
    InheritedDemoObj* accessedIhd = ihd1->m_ptrField;
    EXPECT_EQ( ihd1.get(), accessedIhd );

    ihd1->m_ptrField = caf::Pointer<InheritedDemoObj>( ihd2.get() );
    accessedIhd      = ihd1->m_ptrField;
    EXPECT_EQ( ihd2.get(), accessedIhd );

    // Access
    accessedIhd = ihd1->m_ptrField; // Conversion
    EXPECT_EQ( ihd2.get(), accessedIhd );
    accessedIhd = ihd1->m_ptrField.value();
    EXPECT_EQ( ihd2.get(), accessedIhd );

    caf::Pointer<InheritedDemoObj> accessedPtr;
    EXPECT_EQ( ihd2.get(), accessedIhd );
    accessedPtr = ihd1->m_ptrField();
    EXPECT_EQ( ihd2.get(), accessedPtr.p() );
    accessedPtr = ihd1->m_ptrField();
    EXPECT_EQ( ihd2.get(), accessedPtr.p() );

    // Operator ==
    EXPECT_TRUE( ihd1->m_ptrField == ihd2.get() );
    EXPECT_FALSE( ihd1->m_ptrField == ihd1.get() );

    EXPECT_TRUE( ihd1->m_ptrField == caf::Pointer<InheritedDemoObj>( ihd2.get() ) );

    // Generic access
    {
        std::vector<caf::ObjectHandle*> objects;
        ihd1->m_ptrField.ptrReferencedObjects( &objects );
        EXPECT_EQ( 1u, objects.size() );
        EXPECT_EQ( ihd2.get(), objects[0] );
    }

    // Operator ->
    ihd1->m_ptrField->m_texts = "Hei PtrField";
    EXPECT_TRUE( ihd1->m_ptrField->m_texts == "Hei PtrField" );

    // Referencing system
    {
        std::vector<caf::FieldHandle*> ptrFields;
        ihd2->referringPtrFields( ptrFields );
        EXPECT_EQ( 1u, ptrFields.size() );
        EXPECT_EQ( &( ihd1->m_ptrField ), ptrFields[0] );
    }

    {
        std::vector<caf::ObjectHandle*> objects;
        ihd2->objectsWithReferringPtrFields( objects );
        EXPECT_EQ( 1u, objects.size() );
        EXPECT_EQ( ihd1.get(), objects[0] );
    }

    {
        std::vector<InheritedDemoObj*> reffingDemoObjects;
        ihd2->objectsWithReferringPtrFieldsOfType( reffingDemoObjects );

        EXPECT_EQ( 1u, reffingDemoObjects.size() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of Pointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Pointer )
{
    auto d = std::make_unique<InheritedDemoObj>();

    {
        caf::Pointer<InheritedDemoObj> p;
        EXPECT_TRUE( p == nullptr );
    }

    {
        caf::Pointer<InheritedDemoObj> p( d.get() );
        caf::Pointer<InheritedDemoObj> p2( p );

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

    caf::Pointer<DemoObject> p3( new DemoObject() );

    delete p3;
}
