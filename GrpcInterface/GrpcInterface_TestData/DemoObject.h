//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar AS
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
#pragma once

#include "cafAppEnum.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafNotNull.h"
#include "cafObject.h"
#include "cafObjectMethod.h"

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

    DemoObject();

    ~DemoObject() override {}

    // Fields
    caffa::Field<double>      m_proxyDoubleField;
    caffa::Field<int>         m_proxyIntField;
    caffa::Field<std::string> m_proxyStringField;

    caffa::Field<double>                       doubleField;
    caffa::Field<int>                          intField;
    caffa::Field<int>                          intFieldNonScriptable;
    caffa::Field<std::string>                  stringField;
    caffa::Field<caffa::AppEnum<TestEnumType>> enumField;

    caffa::Field<bool>              boolField;
    caffa::Field<std::vector<bool>> boolVector;

    caffa::Field<std::vector<int>>         intVector;
    caffa::Field<std::vector<std::string>> stringVector;

    caffa::Field<std::vector<double>> doubleVector;
    caffa::Field<std::vector<float>>  floatVector;

private:
    // These are proxy getter/setters and should never be called from client, thus are private
    double getDoubleProxy() const { return m_proxyDoubleValue; }
    void   setDoubleProxy( const double& d ) { m_proxyDoubleValue = d; }

    int  getIntProxy() const { return m_proxyIntValue; }
    void setIntProxy( const int& val ) { m_proxyIntValue = val; }

    std::string getStringProxy() const { return m_proxyStringValue; }
    void        setStringProxy( const std::string& val ) { m_proxyStringValue = val; }

    std::vector<int> getIntVectorProxy() const { return m_proxyIntVector; }
    void             setIntVectorProxy( const std::vector<int>& values ) { m_proxyIntVector = values; }

    std::vector<std::string> getStringVectorProxy() const { return m_proxyStringVector; }
    void setStringVectorProxy( const std::vector<std::string>& values ) { m_proxyStringVector = values; }

    double      m_proxyDoubleValue;
    int         m_proxyIntValue;
    std::string m_proxyStringValue;

    std::vector<int>         m_proxyIntVector;
    std::vector<std::string> m_proxyStringVector;
};

struct DemoObject_copyObjectResult : public caffa::ObjectMethodResult
{
    CAFFA_HEADER_INIT;

    DemoObject_copyObjectResult( bool status = false )
        : caffa::ObjectMethodResult( status )
    {
    }
};

class DemoObject_copyObject : public caffa::ObjectMethod
{
    CAFFA_HEADER_INIT;

public:
    DemoObject_copyObject( caffa::ObjectHandle*      self,
                           double                    doubleValue = -123.0,
                           int                       intValue    = 42,
                           const std::string&        stringValue = "SomeValue",
                           const std::vector<bool>&  boolVector  = {},
                           const std::vector<int>&   intVector   = {},
                           const std::vector<float>& floatVector = {} )
        : caffa::ObjectMethod( self )
    {
        initField( m_doubleField, "doubleArgument" ).withDefault( doubleValue );
        initField( m_intField, "intArgument" ).withDefault( intValue );
        initField( m_stringField, "stringArgument" ).withDefault( stringValue );

        initField( m_boolVector, "boolArrayArgument" ).withDefault( boolVector );
        initField( m_intVector, "intArrayArgument" ).withDefault( intVector );
        initField( m_floatVector, "floatArrayArgument" ).withDefault( floatVector );
    }
    std::unique_ptr<caffa::ObjectMethodResult> execute() override
    {
        CAFFA_DEBUG( "Executing object method on server with values: " << m_doubleField() << ", " << m_intField()
                                                                       << ", " << m_stringField() );
        caffa::not_null<DemoObject*> demoObject = self<DemoObject>();
        demoObject->doubleField                 = m_doubleField();
        demoObject->intField                    = m_intField();
        demoObject->stringField                 = m_stringField();
        demoObject->intVector                   = m_intVector();
        demoObject->boolVector                  = m_boolVector();
        demoObject->floatVector                 = m_floatVector();

        auto demoObjectResult    = std::make_unique<DemoObject_copyObjectResult>();
        demoObjectResult->status = true;
        return std::move( demoObjectResult );
    }
    std::unique_ptr<caffa::ObjectMethodResult> defaultResult() const override
    {
        return std::make_unique<DemoObject_copyObjectResult>();
    }

public:
    caffa::Field<double>      m_doubleField;
    caffa::Field<int>         m_intField;
    caffa::Field<std::string> m_stringField;

    caffa::Field<std::vector<bool>>  m_boolVector;
    caffa::Field<std::vector<int>>   m_intVector;
    caffa::Field<std::vector<float>> m_floatVector;
};

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        initField( m_texts, "Texts" ).withScripting();
        initField( m_childArrayField, "DemoObjectects" ).withScripting();
    }

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};

class DemoDocument : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    DemoDocument()
    {
        initField( demoObject, "DemoObject" ).withScripting();
        initField( m_inheritedDemoObjects, "InheritedDemoObjects" ).withScripting();
        demoObject = std::make_unique<DemoObject>();

        this->setId( "testDocument" );
        this->setFileName( "dummyFileName" );
    }

    void addInheritedObject( std::unique_ptr<InheritedDemoObj> object )
    {
        m_inheritedDemoObjects.push_back( std::move( object ) );
    }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.value(); }

    caffa::ChildField<DemoObject*>            demoObject;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

class DemoDocumentWithNonScriptableMember : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    DemoDocumentWithNonScriptableMember()
    {
        initField( demoObject, "DemoObject" ).withScripting();
        initField( demoObjectNonScriptable, "DemoObjectNonScriptable" );
        initField( m_inheritedDemoObjects, "InheritedDemoObjects" ).withScripting();
        demoObject              = std::make_unique<DemoObject>();
        demoObjectNonScriptable = std::make_unique<DemoObject>();

        this->setId( "testDocument2" );
        this->setFileName( "dummyFileName2" );
    }

    void addInheritedObject( std::unique_ptr<InheritedDemoObj> object )
    {
        m_inheritedDemoObjects.push_back( std::move( object ) );
    }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.value(); }

    caffa::ChildField<DemoObject*>            demoObject;
    caffa::ChildField<DemoObject*>            demoObjectNonScriptable;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};
