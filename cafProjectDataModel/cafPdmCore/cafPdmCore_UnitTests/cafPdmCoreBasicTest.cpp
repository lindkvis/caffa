
#include "gtest/gtest.h"

#include "Parent.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPdmReferenceHelper.h"
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

    ~DemoObject() {}

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
    DemoObject* a = new DemoObject;

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
    DemoObject* a = new DemoObject;

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
    DemoObject* a = new DemoObject;

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
    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    caf::Pointer<DemoObject> s1 = new DemoObject;
    caf::Pointer<DemoObject> s2 = new DemoObject;
    caf::Pointer<DemoObject> s3 = new DemoObject;

    // empty() number 1
    EXPECT_TRUE( ihd1->m_childArrayField.empty() );
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    // push_back()
    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    // Parent field
    EXPECT_EQ( s1->parentField(), &( ihd1->m_childArrayField ) );

    // size()
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( size_t( 3 ), ihd1->m_childArrayField.size() );

    // operator[]
    EXPECT_EQ( s2, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3, ihd1->m_childArrayField[2] );

    // childObjects
    std::vector<caf::ObjectHandle*> objects;
    ihd1->m_childArrayField.childObjects( &objects );
    EXPECT_EQ( size_t( 3 ), objects.size() );

    std::vector<DemoObject*> typedObjects = ihd1->m_childArrayField.childObjects();
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );

    // set()
    ihd1->m_childArrayField.set( 1, NULL );
    EXPECT_TRUE( NULL == ihd1->m_childArrayField[1] );
    EXPECT_TRUE( s2->parentField() == NULL );

    ihd1->m_childArrayField.removeChildObject( NULL );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( s3, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );

    // insert()
    ihd1->m_childArrayField.insert( 1, s2 );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );
    EXPECT_EQ( s2, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s3, ihd1->m_childArrayField[2] );

    EXPECT_TRUE( s2->parentField() == &( ihd1->m_childArrayField ) );

    // erase (index)
    ihd1->m_childArrayField.erase( 1 );
    EXPECT_EQ( size_t( 2 ), ihd1->m_childArrayField.size() );
    EXPECT_EQ( s3, ihd1->m_childArrayField[1] );
    EXPECT_EQ( s1, ihd1->m_childArrayField[0] );

    EXPECT_TRUE( s2->parentField() == NULL );

    // clear()
    ihd1->m_childArrayField.clear();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );

    EXPECT_TRUE( s1->parentField() == NULL );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    ihd1->m_childArrayField.deleteAllChildObjects();
    EXPECT_EQ( size_t( 0 ), ihd1->m_childArrayField.size() );
    EXPECT_TRUE( s1 == NULL );
    EXPECT_TRUE( s2 == NULL );
    EXPECT_TRUE( s3 == NULL );
}

TEST( BaseTest, PdmChildArrayParentField )
{
    // Test of instanciating a class with forward declare of object used in ChildArrayField and ChildField
    Parent* parentObj = new Parent;

    delete parentObj;
}

#include "Child.h"

TEST( BaseTest, PointersFieldInsertVector )
{
    Parent* ihd1 = new Parent;

    Child* s1 = new Child;
    Child* s2 = new Child;
    Child* s3 = new Child;

    std::vector<caf::Pointer<Child>> typedObjects;
    typedObjects.push_back( s1 );
    typedObjects.push_back( s2 );
    typedObjects.push_back( s3 );

    ihd1->m_simpleObjectsField.push_back( new Child );
    ihd1->m_simpleObjectsField.insert( ihd1->m_simpleObjectsField.size(), typedObjects );
    EXPECT_EQ( size_t( 4 ), ihd1->m_simpleObjectsField.size() );
    EXPECT_EQ( ihd1->m_simpleObjectsField[3], s3 );

    delete ihd1;
}

//--------------------------------------------------------------------------------------------------
/// ChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldHandle )
{
    //     virtual size_t      size() const = 0;
    //     virtual bool        empty() const = 0;
    //     virtual void        clear() = 0;
    //     virtual Object*  createAppendObject(int indexAfter) = 0;
    //     virtual void        erase(size_t index) = 0;
    //     virtual void        deleteAllChildObjects() = 0;
    //
    //     virtual Object*  at(size_t index) = 0;
    //
    //     bool                hasSameFieldCountForAllObjects();
    DemoObject* s0          = new DemoObject;
    s0->m_memberDoubleField = 1000;

    DemoObject* s1          = new DemoObject;
    s1->m_memberDoubleField = 1000;

    DemoObject* s2          = new DemoObject;
    s2->m_memberDoubleField = 2000;

    DemoObject* s3          = new DemoObject;
    s3->m_memberDoubleField = 3000;

    InheritedDemoObj*           ihd1      = new InheritedDemoObj;
    caf::ChildArrayFieldHandle* listField = &( ihd1->m_childArrayField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );

    listField->insertAt( 0, s0 );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    EXPECT_EQ( 4u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->erase( 0 );
    EXPECT_EQ( 3u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->deleteAllChildObjects();
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
        explicit A( Child* a )
            : field2( a )
            , b( 0 )
        {
            this->addField( &field2, "field2" );
        }

        ~A() { delete field2(); }

        caf::ChildField<Child*> field2;
        int                     b;
    };

    {
        Child* testValue = new Child;

        // Constructor assignment
        A a( testValue );
        EXPECT_EQ( testValue, a.field2.v() );

        // Guarded
        delete testValue;
        EXPECT_EQ( static_cast<Child*>( nullptr ), a.field2 );
    }
    {
        A      a( NULL );
        Child* c2 = new Child;
        // Assign
        a.field2 = c2;
        // Access
        EXPECT_EQ( c2, a.field2.v() );
        EXPECT_EQ( c2, a.field2 );
        EXPECT_EQ( c2, a.field2.value() );
        EXPECT_TRUE( c2 == a.field2 );

        std::vector<caf::ObjectHandle*> objects;
        a.field2.childObjects( &objects );
        EXPECT_EQ( (size_t)1, objects.size() );
        EXPECT_EQ( c2, objects[0] );
    }
}

TEST( BaseTest, PtrField )
{
    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    InheritedDemoObj* ihd2 = new InheritedDemoObj;

    // Direct access
    EXPECT_EQ( static_cast<InheritedDemoObj*>( nullptr ), ihd1->m_ptrField );

    // Assignment
    ihd1->m_ptrField              = ihd1;
    InheritedDemoObj* accessedIhd = ihd1->m_ptrField;
    EXPECT_EQ( ihd1, accessedIhd );

    ihd1->m_ptrField = caf::Pointer<InheritedDemoObj>( ihd2 );
    accessedIhd      = ihd1->m_ptrField;
    EXPECT_EQ( ihd2, accessedIhd );

    // Access
    accessedIhd = ihd1->m_ptrField; // Conversion
    EXPECT_EQ( ihd2, accessedIhd );
    accessedIhd = ihd1->m_ptrField.value();
    EXPECT_EQ( ihd2, accessedIhd );

    caf::Pointer<InheritedDemoObj> accessedPtr;
    EXPECT_EQ( ihd2, accessedIhd );
    accessedPtr = ihd1->m_ptrField();
    EXPECT_EQ( ihd2, accessedPtr.p() );
    accessedPtr = ihd1->m_ptrField();
    EXPECT_EQ( ihd2, accessedPtr.p() );

    // Operator ==
    EXPECT_TRUE( ihd1->m_ptrField == ihd2 );
    EXPECT_FALSE( ihd1->m_ptrField == ihd1 );

    EXPECT_TRUE( ihd1->m_ptrField == caf::Pointer<InheritedDemoObj>( ihd2 ) );

    // Generic access
    {
        std::vector<caf::ObjectHandle*> objects;
        ihd1->m_ptrField.ptrReferencedObjects( &objects );
        EXPECT_EQ( 1u, objects.size() );
        EXPECT_EQ( ihd2, objects[0] );
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
        EXPECT_EQ( ihd1, objects[0] );
    }

    {
        std::vector<InheritedDemoObj*> reffingDemoObjects;
        ihd2->objectsWithReferringPtrFieldsOfType( reffingDemoObjects );
        EXPECT_EQ( 1u, reffingDemoObjects.size() );
    }

    delete ihd1;
    delete ihd2;
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of Pointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Pointer )
{
    InheritedDemoObj* d = new InheritedDemoObj;

    {
        caf::Pointer<InheritedDemoObj> p;
        EXPECT_TRUE( p == NULL );
    }

    {
        caf::Pointer<InheritedDemoObj> p( d );
        caf::Pointer<InheritedDemoObj> p2( p );

        EXPECT_TRUE( p == d && p2 == d );
        EXPECT_TRUE( p.p() == d );
        p = 0;
        EXPECT_TRUE( p == NULL );
        EXPECT_TRUE( p.isNull() );
        EXPECT_TRUE( p2 == d );
        p = p2;
        EXPECT_TRUE( p == d );
        delete d;
        EXPECT_TRUE( p.isNull() && p2.isNull() );
    }

    caf::Pointer<DemoObject> p3( new DemoObject() );

    delete p3;
}
