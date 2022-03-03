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

#include "gtest.h"
#include <iostream>

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"
#include "cafObjectGroup.h"
#include "cafObservingPointer.h"

#include <fstream>
#include <memory>

/// Demo objects to show the usage of the Pdm system

class SimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    SimpleObj()
        : Object()
        , m_doubleMember( 0.0 )
    {
        initField( m_position, "Position" ).withDefault( 8765.2 );
        initField( m_dir, "Dir" ).withDefault( 123.56 );
        initField( m_up, "Up" ).withDefault( 0.0 );
        initField( m_numbers, "Numbers" );

        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &SimpleObj::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &SimpleObj::doubleMember );

        initField( m_proxyDouble, "ProxyDouble" ).withAccessor( std::move( doubleProxyAccessor ) );
    }

    /// Assignment and copying of CAF objects is not focus for the features. This is only a
    /// "would it work" test
    SimpleObj( const SimpleObj& other )
        : Object()
    {
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

    caffa::Field<double>              m_position;
    caffa::Field<double>              m_dir;
    caffa::Field<double>              m_up;
    caffa::Field<std::vector<double>> m_numbers;
    caffa::DataValueField<double>     m_proxyDouble;

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
CAFFA_SOURCE_INIT( SimpleObj, "SimpleObj", "Object" );

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    DemoObject()
    {
        initField( m_doubleField, "BigNumber" ).withDefault( 0.0 );

        initField( m_intField, "IntNumber" ).withDefault( 0 );

        initField( m_textField, "TextField" ).withDefault( "Test text   end" );
        initField( m_simpleObjPtrField, "SimpleObjPtrFieldWhichIsNull" );
        initField( m_simpleObjPtrField2, "SimpleObjPtrField2" );
        m_simpleObjPtrField2 = std::make_unique<SimpleObj>();
    }

    // Fields
    caffa::Field<double>      m_doubleField;
    caffa::Field<int>         m_intField;
    caffa::Field<std::string> m_textField;

    caffa::ChildField<SimpleObj*> m_simpleObjPtrField;
    caffa::ChildField<SimpleObj*> m_simpleObjPtrField2;
};

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" );

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    InheritedDemoObj()
    {
        initField( m_texts, "Texts" ).withDefault( { "Some", "words" } );
        initField( m_testEnumField, "TestEnumValue" );
        initField( m_simpleObjectsField, "SimpleObjects" );
    }

    caffa::Field<std::vector<std::string>>     m_texts;
    caffa::Field<caffa::AppEnum<TestEnumType>> m_testEnumField;
    caffa::ChildArrayField<SimpleObj*>         m_simpleObjectsField;
};
CAFFA_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj", "DemoObject", "Object" );

class MyDocument : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    MyDocument() { initField( objects, "Objects" ); }

    caffa::ChildArrayField<ObjectHandle*> objects;
};
CAFFA_SOURCE_INIT( MyDocument, "MyDocument", "Document", "Object" );

namespace caffa
{
template <>
void AppEnum<InheritedDemoObj::TestEnumType>::setUp()
{
    addItem( InheritedDemoObj::T1, "T1", "An A letter" );
    addItem( InheritedDemoObj::T2, "T2", "A B letter" );
    addItem( InheritedDemoObj::T3, "T3", "A B letter" );
    setDefault( InheritedDemoObj::T1 );
}

} // namespace caffa

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
    class ObjectWithVectors : public caffa::ObjectHandle
    {
    public:
        ObjectWithVectors()
        {
            this->addField( &field1, "field1" );
            this->addField( &field2, "field2" );
            this->addField( &field3, "field3" );
        }

        std::string classKeywordDynamic() const override { return "ObjectWithVectors"; }

        caffa::Field<std::vector<double>> field1;
        caffa::Field<std::vector<double>> field2;
        caffa::Field<std::vector<double>> field3;
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

//--------------------------------------------------------------------------------------------------
/// Tests the roundtrip: Create, write, read, write and checks that the first and second file are identical
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ReadWrite )
{
    {
        MyDocument doc;

        // Create objects
        auto d1  = std::make_unique<DemoObject>();
        auto d2  = std::make_unique<DemoObject>();
        auto id1 = std::make_unique<InheritedDemoObj>();
        auto id2 = std::make_unique<InheritedDemoObj>();

        auto s1 = std::make_unique<SimpleObj>();
        auto s2 = std::make_unique<SimpleObj>();

        s1->m_numbers = { 1.7 };

        // set some values
        s2->m_numbers = { 2.4, 2.5, 2.6, 2.7 };

        id1->m_texts = { "Hi", "and", "Test with whitespace" };

        d2->m_simpleObjPtrField  = std::move( s2 );
        d2->m_simpleObjPtrField2 = std::move( s1 );

        id1->m_simpleObjectsField.push_back( std::make_unique<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[0]->m_numbers();
            v.push_back( 3.0 );
            id1->m_simpleObjectsField[0]->m_numbers = v;
        }

        id1->m_simpleObjectsField.push_back( std::make_unique<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[1]->m_numbers();
            v.push_back( 3.1 );
            v.push_back( 3.11 );
            v.push_back( 3.12 );
            v.push_back( 3.13 );
            id1->m_simpleObjectsField[1]->m_numbers = v;
        }
        id1->m_simpleObjectsField.push_back( std::make_unique<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[2]->m_numbers();
            v.push_back( 3.2 );
            id1->m_simpleObjectsField[2]->m_numbers = v;
        }

        id1->m_simpleObjectsField.push_back( std::make_unique<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[3]->m_numbers();
            v.push_back( 3.3 );
            id1->m_simpleObjectsField[3]->m_numbers = v;
        }

        // Add to document

        doc.objects.push_back( std::move( d1 ) );
        auto d2p = d2.get();
        doc.objects.push_back( std::move( d2 ) );
        doc.objects.push_back( std::make_unique<SimpleObj>() );
        doc.objects.push_back( std::move( id1 ) );
        doc.objects.push_back( std::move( id2 ) );

        // Write file
        doc.setFileName( "TestFile.json" );
        doc.write();

        {
            std::ifstream f1( doc.fileName() );
            std::string   str1( ( std::istreambuf_iterator<char>( f1 ) ), std::istreambuf_iterator<char>() );
            CAFFA_DEBUG( "Wrote file content to " << doc.fileName() << ":\n" << str1 );
        }
        caffa::ObjectGroup pog;
        for ( size_t i = 0; i < doc.objects.size(); i++ )
        {
            pog.addObject( doc.objects[i] );
        }

        {
            std::vector<caffa::ObservingPointer<DemoObject>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 4 ), demoObjs.size() );
        }
        {
            std::vector<caffa::ObservingPointer<InheritedDemoObj>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 2 ), demoObjs.size() );
        }
        {
            std::vector<caffa::ObservingPointer<SimpleObj>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 1 ), demoObjs.size() );
        }

        d2p->m_simpleObjPtrField = nullptr;
        doc.objects.clear();
    }

    {
        MyDocument doc;

        // Read file
        doc.setFileName( "TestFile.json" );
        ASSERT_TRUE( doc.read() );

        caffa::ObjectGroup pog;
        for ( size_t i = 0; i < doc.objects.size(); i++ )
        {
            pog.addObject( doc.objects[i] );
        }

        // Test sample of that writing actually took place

        std::vector<caffa::ObservingPointer<InheritedDemoObj>> ihDObjs;
        pog.objectsByType( &ihDObjs );
        EXPECT_EQ( size_t( 2 ), ihDObjs.size() );
        ASSERT_EQ( size_t( 4 ), ihDObjs[0]->m_simpleObjectsField.size() );
        ASSERT_EQ( size_t( 4 ), ihDObjs[0]->m_simpleObjectsField[1]->m_numbers().size() );
        EXPECT_EQ( 3.13, ihDObjs[0]->m_simpleObjectsField[1]->m_numbers()[3] );

        EXPECT_EQ( std::string( "Test text   end" ), ihDObjs[0]->m_textField() );

        // Write file
        std::ofstream file( "TestFile2.json" );
        doc.writeStream( file, caffa::JsonSerializer().setSerializeUuids( false ).setSerializeDataTypes( false ) );
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
    auto d = std::make_unique<caffa::Document>();

    {
        caffa::ObservingPointer<caffa::Document> p;
        EXPECT_TRUE( p == nullptr );
    }

    {
        caffa::ObservingPointer<caffa::Document> p( d.get() );
        caffa::ObservingPointer<caffa::Document> p2( p.p() );

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

//--------------------------------------------------------------------------------------------------
/// Tests the Factory
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ObjectFactory )
{
    {
        auto s(
            caffa::dynamic_unique_cast<SimpleObj>( caffa::DefaultObjectFactory::instance()->create( "SimpleObj" ) ) );
        EXPECT_TRUE( s );
    }
    {
        auto s(
            caffa::dynamic_unique_cast<DemoObject>( caffa::DefaultObjectFactory::instance()->create( "DemoObject" ) ) );
        EXPECT_TRUE( s );
    }
    {
        auto s = caffa::dynamic_unique_cast<InheritedDemoObj>(
            caffa::DefaultObjectFactory::instance()->create( "InheritedDemoObj" ) );
        EXPECT_TRUE( s );
    }

    {
        auto s = caffa::dynamic_unique_cast<caffa::Document>(
            caffa::DefaultObjectFactory::instance()->create( "Document" ) );
        EXPECT_TRUE( s );
    }

    {
        auto s = caffa::dynamic_unique_cast<caffa::ObjectGroup>(
            caffa::DefaultObjectFactory::instance()->create( "ObjectGroup" ) );
        EXPECT_TRUE( s );
    }
}

//--------------------------------------------------------------------------------------------------
/// Validate Xml keywords
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ValidXmlKeywords )
{
    EXPECT_TRUE( caffa::ObjectIoCapability::isValidElementName( "Valid_name" ) );

    EXPECT_FALSE( caffa::ObjectIoCapability::isValidElementName( "2Valid_name" ) );
    EXPECT_FALSE( caffa::ObjectIoCapability::isValidElementName( ".Valid_name" ) );
    EXPECT_FALSE( caffa::ObjectIoCapability::isValidElementName( "xml_Valid_name" ) );
    EXPECT_FALSE( caffa::ObjectIoCapability::isValidElementName( "Valid_name_with_space " ) );
}

//--------------------------------------------------------------------------------------------------
/// ChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldHandle )
{
    auto s1        = std::make_unique<SimpleObj>();
    s1->m_position = 1000;
    auto v         = s1->m_numbers();
    v.push_back( 10 );
    s1->m_numbers = v;

    auto s2        = std::make_unique<SimpleObj>();
    s2->m_position = 2000;

    auto s3        = std::make_unique<SimpleObj>();
    s3->m_position = 3000;

    auto                          ihd1      = std::make_unique<InheritedDemoObj>();
    caffa::ChildArrayFieldHandle* listField = &( ihd1->m_simpleObjectsField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( std::make_unique<SimpleObj>() );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( std::move( s1 ) );
    ihd1->m_simpleObjectsField.push_back( std::move( s2 ) );
    ihd1->m_simpleObjectsField.push_back( std::move( s3 ) );

    EXPECT_EQ( 4u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    listField->erase( 0 );
    EXPECT_EQ( 3u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    listField->clear();
    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->empty() );
}

TEST( BaseTest, Uuids )
{
    auto obj1 = std::make_unique<SimpleObj>();
    EXPECT_FALSE( obj1->uuid().empty() );
    CAFFA_INFO( "UUID: " << obj1->uuid() );
    // Should never have collisions.
    for ( size_t i = 0; i < 10000; ++i )
    {
        auto obj2 = std::make_unique<SimpleObj>();
        EXPECT_FALSE( obj2->uuid().empty() );
        EXPECT_NE( obj1->uuid(), obj2->uuid() );
    }
}
