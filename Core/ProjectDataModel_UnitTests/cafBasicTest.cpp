// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################

#include "gtest.h"
#include <iostream>

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"

#include <fstream>
#include <functional>
#include <memory>

using namespace std::placeholders;

/// Demo objects to show the usage of the Pdm system

class SimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT( SimpleObj, Object )

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
        doubleProxyAccessor->registerSetMethod( std::bind( &SimpleObj::setDoubleMember, this, _1 ) );
        doubleProxyAccessor->registerGetMethod( std::bind( &SimpleObj::doubleMember, this ) );

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

        m_position     = other.m_position();
        m_dir          = other.m_dir();
        m_up           = other.m_up();
        m_numbers      = other.m_numbers();
        m_doubleMember = other.m_doubleMember;
    }

    ~SimpleObj() {}

    caffa::Field<double>              m_position;
    caffa::Field<double>              m_dir;
    caffa::Field<double>              m_up;
    caffa::Field<std::vector<double>> m_numbers;
    caffa::Field<double>              m_proxyDouble;

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
CAFFA_SOURCE_INIT( SimpleObj )

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT( DemoObject, Object )

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

CAFFA_SOURCE_INIT( DemoObject )

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT( InheritedDemoObj, DemoObject )

public:
    InheritedDemoObj()
    {
        initField( m_texts, "Texts" ).withDefault( { "Some", "words" } );
        initField( m_simpleObjectsField, "SimpleObjects" );
    }

    caffa::Field<std::vector<std::string>> m_texts;
    caffa::ChildArrayField<SimpleObj*>     m_simpleObjectsField;
};
CAFFA_SOURCE_INIT( InheritedDemoObj )

class MyDocument : public caffa::Document
{
    CAFFA_HEADER_INIT( MyDocument, Document )

public:
    MyDocument() { initField( objects, "Objects" ); }

    caffa::ChildArrayField<ObjectHandle*> objects;
};
CAFFA_SOURCE_INIT( MyDocument )

TEST( BaseTest, Delete )
{
    SimpleObj* s2 = new SimpleObj;
    delete s2;
}

class ObjectWithVectors : public caffa::Object
{
    CAFFA_HEADER_INIT( ObjectWithVectors, Object )

public:
    ObjectWithVectors()
    {
        initField( field1, "field1" );
        initField( field2, "field2" );
        initField( field3, "field3" );
    }

    caffa::Field<std::vector<double>> field1;
    caffa::Field<std::vector<double>> field2;
    caffa::Field<std::vector<double>> field3;
};

CAFFA_SOURCE_INIT( ObjectWithVectors );

struct Test2
{
};

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

    ObjectWithVectors myObj;

    // Constructors
    myObj.field2 = testValue;
    EXPECT_EQ( 1.3, myObj.field2.value()[2] );

    myObj.field3 = myObj.field2();
    EXPECT_EQ( 1.3, myObj.field3.value()[2] );
    EXPECT_EQ( size_t( 0 ), myObj.field1().size() );

    // Operators
    EXPECT_FALSE( myObj.field1() == myObj.field3() );
    myObj.field1 = myObj.field2();
    EXPECT_EQ( 1.3, myObj.field1()[2] );
    myObj.field1 = testValue2;
    EXPECT_EQ( 2.3, myObj.field1()[2] );
    myObj.field3 = myObj.field1();
    EXPECT_TRUE( myObj.field1() == myObj.field3() );
}

//--------------------------------------------------------------------------------------------------
/// Tests the roundtrip: Create, write, read, write and checks that the first and second file are identical
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ReadWrite )
{
    {
        MyDocument doc;

        // Create objects
        auto d1  = std::make_shared<DemoObject>();
        auto d2  = std::make_shared<DemoObject>();
        auto id1 = std::make_shared<InheritedDemoObj>();
        auto id2 = std::make_shared<InheritedDemoObj>();

        auto s1 = std::make_shared<SimpleObj>();
        auto s2 = std::make_shared<SimpleObj>();

        s1->m_numbers = { 1.7 };

        // set some values
        s2->m_numbers = { 2.4, 2.5, 2.6, 2.7 };

        id1->m_texts = { "Hi", "and", "Test with whitespace" };

        d2->m_simpleObjPtrField  = s2;
        d2->m_simpleObjPtrField2 = s1;

        id1->m_simpleObjectsField.push_back( std::make_shared<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[0]->m_numbers();
            v.push_back( 3.0 );
            id1->m_simpleObjectsField[0]->m_numbers = v;
        }

        id1->m_simpleObjectsField.push_back( std::make_shared<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[1]->m_numbers();
            v.push_back( 3.1 );
            v.push_back( 3.11 );
            v.push_back( 3.12 );
            v.push_back( 3.13 );
            id1->m_simpleObjectsField[1]->m_numbers = v;
        }
        id1->m_simpleObjectsField.push_back( std::make_shared<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[2]->m_numbers();
            v.push_back( 3.2 );
            id1->m_simpleObjectsField[2]->m_numbers = v;
        }

        id1->m_simpleObjectsField.push_back( std::make_shared<SimpleObj>() );
        {
            auto v = id1->m_simpleObjectsField[3]->m_numbers();
            v.push_back( 3.3 );
            id1->m_simpleObjectsField[3]->m_numbers = v;
        }

        // Add to document

        doc.objects.push_back( d1 );
        auto d2p = d2.get();
        doc.objects.push_back( d2 );
        doc.objects.push_back( std::make_shared<SimpleObj>() );
        doc.objects.push_back( id1 );
        doc.objects.push_back( id2 );

        // Write file
        doc.setFileName( "TestFile.json" );
        doc.writeToJsonFile();

        {
            std::ifstream f1( doc.fileName() );
            std::string   str1( ( std::istreambuf_iterator<char>( f1 ) ), std::istreambuf_iterator<char>() );
            CAFFA_DEBUG( "Wrote file content to " << doc.fileName() << ":\n" << str1 );
        }

        d2p->m_simpleObjPtrField = nullptr;
        doc.objects.clear();
    }

    {
        MyDocument doc;

        // Read file
        doc.setFileName( "TestFile.json" );
        ASSERT_TRUE( doc.readFromJsonFile() );

        // Write file
        std::ofstream file( "TestFile2.json" );
        caffa::JsonSerializer().setSerializeUuids( false ).writeStream( &doc, file );
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
/// Tests the Factory
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ObjectFactory )
{
    {
        auto s(
            std::dynamic_pointer_cast<SimpleObj>( caffa::DefaultObjectFactory::instance()->create( "SimpleObj" ) ) );
        EXPECT_TRUE( s );
    }
    {
        auto s(
            std::dynamic_pointer_cast<DemoObject>( caffa::DefaultObjectFactory::instance()->create( "DemoObject" ) ) );
        EXPECT_TRUE( s );
    }
    {
        auto s = std::dynamic_pointer_cast<InheritedDemoObj>(
            caffa::DefaultObjectFactory::instance()->create( "InheritedDemoObj" ) );
        EXPECT_TRUE( s );
    }

    {
        auto s =
            std::dynamic_pointer_cast<caffa::Document>( caffa::DefaultObjectFactory::instance()->create( "Document" ) );
        EXPECT_TRUE( s );
    }
}

//--------------------------------------------------------------------------------------------------
/// Validate Object keywords
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ValidObjectKeywords )
{
    EXPECT_TRUE( caffa::ObjectHandle::isValidKeyword( "Valid_name" ) );

    EXPECT_FALSE( caffa::ObjectHandle::isValidKeyword( "2Valid_name" ) );
    EXPECT_FALSE( caffa::ObjectHandle::isValidKeyword( ".Valid_name" ) );
    EXPECT_TRUE( caffa::ObjectHandle::isValidKeyword( "xml_Valid_name" ) );
    EXPECT_FALSE( caffa::ObjectHandle::isValidKeyword( "Valid_name_with_space " ) );
}

//--------------------------------------------------------------------------------------------------
/// ChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldHandle )
{
    auto s1        = std::make_shared<SimpleObj>();
    s1->m_position = 1000;
    auto v         = s1->m_numbers();
    v.push_back( 10 );
    s1->m_numbers = v;

    auto s2        = std::make_shared<SimpleObj>();
    s2->m_position = 2000;

    auto s3        = std::make_shared<SimpleObj>();
    s3->m_position = 3000;

    auto                          ihd1      = std::make_shared<InheritedDemoObj>();
    caffa::ChildArrayFieldHandle* listField = &( ihd1->m_simpleObjectsField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( std::make_shared<SimpleObj>() );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( s1 );
    ihd1->m_simpleObjectsField.push_back( s2 );
    ihd1->m_simpleObjectsField.push_back( s3 );

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
    auto obj1 = std::make_shared<SimpleObj>();
    EXPECT_FALSE( obj1->uuid().empty() );
    CAFFA_INFO( "UUID: " << obj1->uuid() );
    // Should never have collisions.
    for ( size_t i = 0; i < 2000; ++i )
    {
        auto obj2 = std::make_shared<SimpleObj>();
        EXPECT_FALSE( obj2->uuid().empty() );
        EXPECT_NE( obj1->uuid(), obj2->uuid() );
    }
}
