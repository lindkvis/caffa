// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- 3D-Radar AS
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
#pragma once

#include "cafAppEnum.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafMethod.h"
#include "cafNotNull.h"
#include "cafObject.h"

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT( DemoObject, Object )

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    DemoObject();

    ~DemoObject() noexcept override {}

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

    caffa::Method<void( int, double, std::string )> copyValues;

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

    void _copyValues( int intValue, double doubleValue, std::string stringValue );

    double      m_proxyDoubleValue;
    int         m_proxyIntValue;
    std::string m_proxyStringValue;

    std::vector<int>         m_proxyIntVector;
    std::vector<std::string> m_proxyStringVector;
};

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT( InheritedDemoObj, DemoObject )

public:
    InheritedDemoObj()
    {
        initField( m_texts, "texts" ).withScripting();
        initField( m_childArrayField, "demoObjects" ).withScripting();
    }

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};

class DemoDocument : public caffa::Document
{
    CAFFA_HEADER_INIT( DemoDocument, Document )

public:
    DemoDocument()
    {
        initField( demoObject, "demoObject" ).withScripting();
        initField( m_inheritedDemoObjects, "inheritedDemoObjects" ).withScripting();
        demoObject = std::make_shared<DemoObject>();

        this->setId( "testDocument" );
        this->setFileName( "dummyFileName" );
    }

    void addInheritedObject( std::shared_ptr<InheritedDemoObj> object ) { m_inheritedDemoObjects.push_back( object ); }
    std::vector<std::shared_ptr<InheritedDemoObj>> inheritedObjects() { return m_inheritedDemoObjects; }

    caffa::ChildField<DemoObject*>            demoObject;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

class DemoDocumentWithNonScriptableMember : public caffa::Document
{
    CAFFA_HEADER_INIT( DemoDocumentWithNonScriptableMember, Document )

public:
    DemoDocumentWithNonScriptableMember()
    {
        initField( demoObject, "demoObject" ).withScripting();
        initField( demoObjectNonScriptable, "demoObjectNonScriptable" );
        initField( m_inheritedDemoObjects, "inheritedDemoObjects" ).withScripting();
        demoObject              = std::make_shared<DemoObject>();
        demoObjectNonScriptable = std::make_shared<DemoObject>();

        this->setId( "testDocument2" );
        this->setFileName( "dummyFileName2" );
    }

    void addInheritedObject( std::shared_ptr<InheritedDemoObj> object ) { m_inheritedDemoObjects.push_back( object ); }
    std::vector<std::shared_ptr<InheritedDemoObj>> inheritedObjects() { return m_inheritedDemoObjects; }

    caffa::ChildField<DemoObject*>            demoObject;
    caffa::ChildField<DemoObject*>            demoObjectNonScriptable;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};
