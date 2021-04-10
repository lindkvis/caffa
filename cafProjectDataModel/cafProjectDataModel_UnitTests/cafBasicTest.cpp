//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "gtest/gtest.h"
#include <iostream>

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObject.h"
#include "cafObjectGroup.h"
#include "cafPointer.h"
#include "cafReferenceHelper.h"

#include <fstream>
#include <memory>

/// Demo objects to show the usage of the Pdm system

class SimpleObj : public caf::Object
{
    CAF_HEADER_INIT;

public:
    SimpleObj()
        : Object()
        , m_doubleMember( 0.0 )
    {
        initObject();
        initField( m_position, "Position" ).withDefault( 8765.2 );
        initField( m_dir, "Dir" ).withDefault( 123.56 );
        initField( m_up, "Up" ).withDefault( 0.0 );
        initField( m_numbers, "Numbers" );

        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &SimpleObj::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &SimpleObj::doubleMember );

        initField( m_proxyDouble, "ProxyDouble" ).withAccessor( std::move( doubleProxyAccessor ) );
    }

    /// Assignment and copying of CAF objects is not focus for the features. This is only a
    /// "would it work" test
    SimpleObj( const SimpleObj& other )
        : Object()
    {
        initObject();
        initField( m_position, "Position" );
        initField( m_dir, "Dir" );
        initField( m_up, "Up" );
        initField( m_numbers, "Numbers" );

        m_position     = other.m_position;
        m_dir          = other.m_dir;
        m_up           = other.m_up;
        m_numbers      = other.m_numbers;
        m_doubleMember = other.m_doubleMember;
    }

    ~SimpleObj() {}

    caf::Field<double>              m_position;
    caf::Field<double>              m_dir;
    caf::Field<double>              m_up;
    caf::Field<std::vector<double>> m_numbers;
    caf::DataValueField<double>     m_proxyDouble;

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
CAF_SOURCE_INIT( SimpleObj, "SimpleObj" );

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject()
    {
        initObject();

        initField( m_doubleField, "BigNumber" ).withDefault( 0.0 );

        initField( m_intField, "IntNumber" ).withDefault( 0 );

        initField( m_textField, "TextField" ).withDefault( "Test text   end" );
        initField( m_simpleObjPtrField, "SimpleObjPtrField" );
        initField( m_simpleObjPtrField2, "SimpleObjPtrField2" );
        m_simpleObjPtrField2 = new SimpleObj;
    }

    // Fields
    caf::Field<double>      m_doubleField;
    caf::Field<int>         m_intField;
    caf::Field<std::string> m_textField;

    caf::ChildField<SimpleObj*> m_simpleObjPtrField;
    caf::ChildField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

class InheritedDemoObj : public DemoObject
{
    CAF_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    InheritedDemoObj()
    {
        initObject();

        initField( m_texts, "Texts" ).withDefault( { "Some", "words" } );
        initField( m_testEnumField, "TestEnumValue" );
        initField( m_simpleObjectsField, "SimpleObjects" );
    }

    caf::Field<std::vector<std::string>>   m_texts;
    caf::Field<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::ChildArrayField<SimpleObj*>       m_simpleObjectsField;
};
CAF_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj" );

class MyDocument : public caf::Document
{
    CAF_HEADER_INIT;

public:
    MyDocument()
    {
        initObject();
        initField( objects, "Objects" );
    }

    caf::ChildArrayField<ObjectHandle*> objects;
};
CAF_SOURCE_INIT( MyDocument, "MyDocument" );

namespace caf
{
template <>
void AppEnum<InheritedDemoObj::TestEnumType>::setUp()
{
    addItem( InheritedDemoObj::T1, "T1", "An A letter" );
    addItem( InheritedDemoObj::T2, "T2", "A B letter" );
    addItem( InheritedDemoObj::T3, "T3", "A B letter" );
    setDefault( InheritedDemoObj::T1 );
}

} // namespace caf

TEST( BaseTest, Delete )
{
    SimpleObj* s2 = new SimpleObj;
    delete s2;
}

//--------------------------------------------------------------------------------------------------
/// Test of Field operations
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, NormalField )
{
    class ObjectWithVectors : public caf::ObjectHandle
    {
    public:
        ObjectWithVectors()
        {
            this->addField( &field1, "field1" );
            this->addField( &field2, "field2" );
            this->addField( &field3, "field3" );
        }

        caf::Field<std::vector<double>> field1;
        caf::Field<std::vector<double>> field2;
        caf::Field<std::vector<double>> field3;
    };

    std::vector<double> testValue;
    testValue.push_back( 1.1 );
    testValue.push_back( 1.2 );
    testValue.push_back( 1.3 );

    std::vector<double> testValue2;
    testValue2.push_back( 2.1 );
    testValue2.push_back( 2.2 );
    testValue2.push_back( 2.3 );

    ObjectWithVectors myObj;

    // Constructors
    myObj.field2 = testValue;
    EXPECT_EQ( 1.3, myObj.field2.value()[2] );

    myObj.field3 = myObj.field2;
    EXPECT_EQ( 1.3, myObj.field3.value()[2] );
    EXPECT_EQ( size_t( 0 ), myObj.field1().size() );

    // Operators
    EXPECT_FALSE( myObj.field1 == myObj.field3 );
    myObj.field1 = myObj.field2;
    EXPECT_EQ( 1.3, myObj.field1()[2] );
    myObj.field1 = testValue2;
    EXPECT_EQ( 2.3, myObj.field1()[2] );
    myObj.field3 = myObj.field1;
    EXPECT_TRUE( myObj.field1 == myObj.field3 );
}

#if 0
//--------------------------------------------------------------------------------------------------
/// Test of Field of pointer operations
//--------------------------------------------------------------------------------------------------

TEST(BaseTest, PointerField)
{
    SimpleObj* testValue = new SimpleObj;
    testValue->m_numbers.v().push_back(1.1);
    testValue->m_numbers.v().push_back(1.2);
    testValue->m_numbers.v().push_back(1.3);

    SimpleObj* testValue2 = new SimpleObj;
    testValue->m_numbers.v().push_back(2.1);
    testValue->m_numbers.v().push_back(2.2);
    testValue->m_numbers.v().push_back(2.3);

    // Constructors
    caf::Field<SimpleObj*> field2(testValue);
    EXPECT_EQ(testValue, field2.v());
    caf::Field<SimpleObj*> field3(field2);
    EXPECT_EQ(testValue, field3.v());
    caf::Field<SimpleObj*> field1;
    EXPECT_EQ((SimpleObj*)0, field1.v());

    // Operators
    EXPECT_FALSE(field1 == field3);
    field1 = field2;
    EXPECT_EQ(testValue, field1);
    field1 = testValue2;
    field3 = testValue2;
    EXPECT_EQ(testValue2, field1);
    EXPECT_TRUE(field1 == field3);
    delete testValue;
    delete testValue2;
    EXPECT_EQ((SimpleObj*)0, field1);
    EXPECT_EQ((SimpleObj*)0, field2);
    EXPECT_EQ((SimpleObj*)0, field3);
}
#endif

//--------------------------------------------------------------------------------------------------
/// Test of PointersField operations
//--------------------------------------------------------------------------------------------------
#if 0
TEST(BaseTest, ChildArrayField)
{
    std::vector<caf::FieldHandle*> parentFields;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    SimpleObj* s1 = new SimpleObj;
    SimpleObj* s2 = new SimpleObj;
    SimpleObj* s3 = new SimpleObj;

    // empty() number 1
    EXPECT_TRUE(ihd1->m_simpleObjectsField.empty());
    EXPECT_EQ(size_t(0), ihd1->m_simpleObjectsField.size());

    // push_back()
    ihd1->m_simpleObjectsField.push_back(s1);
    ihd1->m_simpleObjectsField.push_back(s2);
    ihd1->m_simpleObjectsField.push_back(s3);

    s1->parentField(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // size()
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());

    // operator[]
    EXPECT_EQ(s2, ihd1->m_simpleObjectsField[1]);
    EXPECT_EQ(s3, ihd1->m_simpleObjectsField[2]);

    // childObjects
    std::vector<caf::ObjectHandle*> objects;
    ihd1->m_simpleObjectsField.childObjects(&objects);
    EXPECT_EQ(size_t(3), objects.size());

    // Operator ==, Operator =
    InheritedDemoObj* ihd2 = new InheritedDemoObj;
    EXPECT_FALSE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 
    ihd2->m_simpleObjectsField = ihd1->m_simpleObjectsField;
    EXPECT_TRUE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 

    s1->parentFields(parentFields);
    EXPECT_EQ(size_t(2),  parentFields.size());
    parentFields.clear();

    // set(), Operator=
    ihd2->m_simpleObjectsField.set(1, NULL);
    EXPECT_FALSE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 
    EXPECT_TRUE(NULL == ihd2->m_simpleObjectsField[1]);

    s2->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // removeAll(pointer)
    ihd2->m_simpleObjectsField.removeChildObject(NULL);
    EXPECT_EQ(size_t(2), ihd2->m_simpleObjectsField.size());
    EXPECT_EQ(s3, ihd2->m_simpleObjectsField[1]);
    EXPECT_EQ(s1, ihd2->m_simpleObjectsField[0]);

    // insert()
    ihd2->m_simpleObjectsField.insert(1, s2);
    EXPECT_TRUE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 

    s2->parentFields(parentFields);
    EXPECT_EQ(size_t(2),  parentFields.size());
    parentFields.clear();

    // erase (index)
    ihd2->m_simpleObjectsField.erase(1);
    EXPECT_EQ(size_t(2),  ihd2->m_simpleObjectsField.size());
    EXPECT_EQ(s3, ihd2->m_simpleObjectsField[1]);
    EXPECT_EQ(s1, ihd2->m_simpleObjectsField[0]);

    s2->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // clear()
    ihd2->m_simpleObjectsField.clear();
    EXPECT_EQ(size_t(0),  ihd2->m_simpleObjectsField.size());

    s1->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

 
}
#endif

void PrintTo( const std::string& val, std::ostream* os )
{
    *os << val;
}

//--------------------------------------------------------------------------------------------------
/// Tests the roundtrip: Create, write, read, write and checks that the first and second file are identical
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ReadWrite )
{
    std::string xmlDocumentContentWithErrors;

    {
        MyDocument doc;

        // Create objects
        DemoObject*       d1  = new DemoObject;
        DemoObject*       d2  = new DemoObject;
        InheritedDemoObj* id1 = new InheritedDemoObj;
        InheritedDemoObj* id2 = new InheritedDemoObj;

        SimpleObj* s1 = new SimpleObj;
        SimpleObj  s2;

        s1->m_numbers = { 1.7 };

        // set some values
        s2.m_numbers = { 2.4, 2.5, 2.6, 2.7 };

        id1->m_texts = { "Hi", "and", "Test with whitespace" };

        d2->m_simpleObjPtrField  = &s2;
        d2->m_simpleObjPtrField2 = s1;

        id1->m_simpleObjectsField.push_back( new SimpleObj );
        {
            auto v = id1->m_simpleObjectsField[0]->m_numbers();
            v.push_back( 3.0 );
            id1->m_simpleObjectsField[0]->m_numbers = v;
        }

        id1->m_simpleObjectsField.push_back( new SimpleObj );
        {
            auto v = id1->m_simpleObjectsField[1]->m_numbers();
            v.push_back( 3.1 );
            v.push_back( 3.11 );
            v.push_back( 3.12 );
            v.push_back( 3.13 );
            id1->m_simpleObjectsField[1]->m_numbers = v;
        }
        id1->m_simpleObjectsField.push_back( new SimpleObj );
        {
            auto v = id1->m_simpleObjectsField[2]->m_numbers();
            v.push_back( 3.2 );
            id1->m_simpleObjectsField[2]->m_numbers = v;
        }

        id1->m_simpleObjectsField.push_back( new SimpleObj );
        {
            auto v = id1->m_simpleObjectsField[3]->m_numbers();
            v.push_back( 3.3 );
            id1->m_simpleObjectsField[3]->m_numbers = v;
        }

        // Add to document

        doc.objects.push_back( d1 );
        doc.objects.push_back( d2 );
        doc.objects.push_back( new SimpleObj );
        doc.objects.push_back( id1 );
        doc.objects.push_back( id2 );

        // Write file
        doc.fileName = "TestFile.json";
        doc.write();

        caf::ObjectGroup pog;
        for ( size_t i = 0; i < doc.objects.size(); i++ )
        {
            pog.addObject( doc.objects[i] );
        }

        {
            std::vector<caf::Pointer<DemoObject>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 4 ), demoObjs.size() );
        }
        {
            std::vector<caf::Pointer<InheritedDemoObj>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 2 ), demoObjs.size() );
        }
        {
            std::vector<caf::Pointer<SimpleObj>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 1 ), demoObjs.size() );
        }

        d2->m_simpleObjPtrField = NULL;
        doc.objects.deleteAllChildObjects();
    }

    {
        MyDocument doc;

        // Read file
        doc.fileName = "TestFile.json";
        ASSERT_TRUE( doc.read() );

        caf::ObjectGroup pog;
        for ( size_t i = 0; i < doc.objects.size(); i++ )
        {
            pog.addObject( doc.objects[i] );
        }

        // Test sample of that writing actually took place

        std::vector<caf::Pointer<InheritedDemoObj>> ihDObjs;
        pog.objectsByType( &ihDObjs );
        EXPECT_EQ( size_t( 2 ), ihDObjs.size() );
        ASSERT_EQ( size_t( 4 ), ihDObjs[0]->m_simpleObjectsField.size() );
        ASSERT_EQ( size_t( 4 ), ihDObjs[0]->m_simpleObjectsField[1]->m_numbers().size() );
        EXPECT_EQ( 3.13, ihDObjs[0]->m_simpleObjectsField[1]->m_numbers()[3] );

        EXPECT_EQ( std::string( "Test text   end" ), ihDObjs[0]->m_textField() );

        // Write file
        std::ofstream file( "TestFile2.json" );
        doc.writeFile( file );
        file.close();
    }

    // Check that the files are identical
    {
        std::ifstream f1( "TestFile.json" );
        std::ifstream f2( "TestFile2.json" );
        std::string   str1( ( std::istreambuf_iterator<char>( f1 ) ), std::istreambuf_iterator<char>() );
        std::string   str2( ( std::istreambuf_iterator<char>( f2 ) ), std::istreambuf_iterator<char>() );

        bool equal = str1 == str2;
        EXPECT_TRUE( equal );
    }
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of Pointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Pointer )
{
    caf::Document* d = new caf::Document;

    {
        caf::Pointer<caf::Document> p;
        EXPECT_TRUE( p == NULL );
    }

    {
        caf::Pointer<caf::Document> p( d );
        caf::Pointer<caf::Document> p2( p );

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

//--------------------------------------------------------------------------------------------------
/// Tests the Factory
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ObjectFactory )
{
    {
        SimpleObj* s = dynamic_cast<SimpleObj*>( caf::DefaultObjectFactory::instance()->create( "SimpleObj" ) );
        EXPECT_TRUE( s != NULL );
    }
    {
        DemoObject* s = dynamic_cast<DemoObject*>( caf::DefaultObjectFactory::instance()->create( "DemoObject" ) );
        EXPECT_TRUE( s != NULL );
        delete s;
    }
    {
        InheritedDemoObj* s =
            dynamic_cast<InheritedDemoObj*>( caf::DefaultObjectFactory::instance()->create( "InheritedDemoObj" ) );
        EXPECT_TRUE( s != NULL );
    }

    {
        caf::Document* s = dynamic_cast<caf::Document*>( caf::DefaultObjectFactory::instance()->create( "Document" ) );
        EXPECT_TRUE( s != NULL );
    }

    {
        caf::ObjectGroup* s =
            dynamic_cast<caf::ObjectGroup*>( caf::DefaultObjectFactory::instance()->create( "ObjectGroup" ) );
        EXPECT_TRUE( s != NULL );
    }
}

//--------------------------------------------------------------------------------------------------
/// Validate Xml keywords
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ValidXmlKeywords )
{
    EXPECT_TRUE( caf::ObjectIoCapability::isValidElementName( "Valid_name" ) );

    EXPECT_FALSE( caf::ObjectIoCapability::isValidElementName( "2Valid_name" ) );
    EXPECT_FALSE( caf::ObjectIoCapability::isValidElementName( ".Valid_name" ) );
    EXPECT_FALSE( caf::ObjectIoCapability::isValidElementName( "xml_Valid_name" ) );
    EXPECT_FALSE( caf::ObjectIoCapability::isValidElementName( "Valid_name_with_space " ) );
}

TEST( BaseTest, PointersFieldInsertVector )
{
    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    SimpleObj* s1 = new SimpleObj;
    SimpleObj* s2 = new SimpleObj;
    SimpleObj* s3 = new SimpleObj;

    caf::ObjectGroup group;
    group.addObject( s1 );
    group.addObject( s2 );
    group.addObject( s3 );

    std::vector<caf::Pointer<SimpleObj>> typedObjects;
    group.objectsByType( &typedObjects );
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );

    std::vector<caf::Pointer<SimpleObj>> objs;
    objs.push_back( new SimpleObj );
    objs.push_back( new SimpleObj );
    objs.push_back( new SimpleObj );

    ihd1->m_simpleObjectsField.insert( ihd1->m_simpleObjectsField.size(), objs );
    EXPECT_EQ( size_t( 3 ), ihd1->m_simpleObjectsField.size() );

    delete ihd1;
}

TEST( BaseTest, ObjectGroupCopyOfTypedObjects )
{
    SimpleObj* s1  = new SimpleObj;
    s1->m_position = 1000;
    auto v         = s1->m_numbers();
    v.push_back( 10 );
    s1->m_numbers = v;

    SimpleObj* s2  = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3  = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    caf::ObjectGroup og;
    og.objects.push_back( s1 );
    og.objects.push_back( s2 );
    og.objects.push_back( s3 );
    og.objects.push_back( ihd1 );

    std::vector<caf::Pointer<SimpleObj>> simpleObjList;
    og.createCopyByType( &simpleObjList, caf::DefaultObjectFactory::instance() );
    EXPECT_EQ( size_t( 3 ), simpleObjList.size() );
    EXPECT_EQ( 1000.0, simpleObjList[0]->m_position() );
    EXPECT_EQ( size_t( 1 ), simpleObjList[0]->m_numbers.value().size() );
    EXPECT_EQ( 10.0, simpleObjList[0]->m_numbers.value()[0] );

    EXPECT_EQ( 2000.0, simpleObjList[1]->m_position() );
    EXPECT_EQ( 3000.0, simpleObjList[2]->m_position() );

    std::vector<caf::Pointer<InheritedDemoObj>> inheritObjList;
    og.createCopyByType( &inheritObjList, caf::DefaultObjectFactory::instance() );
    EXPECT_EQ( size_t( 1 ), inheritObjList.size() );

    og.deleteObjects();
    EXPECT_EQ( size_t( 3 ), simpleObjList.size() );
    EXPECT_EQ( size_t( 1 ), inheritObjList.size() );
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

    SimpleObj* s1  = new SimpleObj;
    s1->m_position = 1000;
    auto v         = s1->m_numbers();
    v.push_back( 10 );
    s1->m_numbers = v;

    SimpleObj* s2  = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3  = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj*           ihd1      = new InheritedDemoObj;
    caf::ChildArrayFieldHandle* listField = &( ihd1->m_simpleObjectsField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( new SimpleObj );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( s1 );
    ihd1->m_simpleObjectsField.push_back( s2 );
    ihd1->m_simpleObjectsField.push_back( s3 );

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

class ReferenceDemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    ReferenceDemoObject()
    {
        initObject();

        initField( m_pointersField, "SimpleObjPtrField" );
        initField( m_simpleObjPtrField2, "SimpleObjPtrField2" );
    }

    // Fields
    caf::ChildField<ObjectHandle*>   m_pointersField;
    caf::ChildArrayField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_SOURCE_INIT( ReferenceDemoObject, "ReferenceDemoObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ReferenceHelper )
{
    SimpleObj* s1  = new SimpleObj;
    s1->m_position = 1000;
    auto v         = s1->m_numbers();
    v.push_back( 10 );
    s1->m_numbers = v;

    SimpleObj* s2  = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3  = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    ihd1->m_simpleObjectsField.push_back( new SimpleObj );

    ihd1->m_simpleObjectsField.push_back( s1 );
    ihd1->m_simpleObjectsField.push_back( s2 );
    ihd1->m_simpleObjectsField.push_back( s3 );

    {
        std::string refString = caf::ReferenceHelper::referenceFromRootToObject( NULL, s3 );
        EXPECT_TRUE( refString.empty() );

        refString                  = caf::ReferenceHelper::referenceFromRootToObject( ihd1, s3 );
        std::string expectedString = ihd1->m_simpleObjectsField.keyword() + " 3";
        EXPECT_STREQ( refString.c_str(), expectedString.c_str() );

        caf::ObjectHandle* fromRef = caf::ReferenceHelper::objectFromReference( ihd1, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    ReferenceDemoObject* objA = new ReferenceDemoObject;
    objA->m_pointersField     = ihd1;

    {
        std::string refString = caf::ReferenceHelper::referenceFromRootToObject( objA, s3 );

        caf::ObjectHandle* fromRef = caf::ReferenceHelper::objectFromReference( objA, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    // Test reference to field
    {
        std::string refString = caf::ReferenceHelper::referenceFromRootToField( objA, &( ihd1->m_simpleObjectsField ) );

        caf::FieldHandle* fromRef = caf::ReferenceHelper::fieldFromReference( objA, refString );
        EXPECT_TRUE( fromRef == &( ihd1->m_simpleObjectsField ) );
    }
}
