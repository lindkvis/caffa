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

using namespace caffa;

class DemoObject : public Object
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
    Field<double>      m_proxyDoubleField;
    Field<int>         m_proxyIntField;
    Field<std::string> m_proxyStringField;

    Field<double>                doubleField;
    Field<int>                   intField;
    Field<int>                   intFieldNonScriptable;
    Field<std::string>           stringField;
    Field<AppEnum<TestEnumType>> enumField;

    Field<bool>              boolField;
    Field<std::vector<bool>> boolVector;

    Field<std::vector<int>>         intVector;
    Field<std::vector<std::string>> stringVector;

    Field<std::vector<double>> doubleVector;
    Field<std::vector<float>>  floatVector;

    Method<void( int, double, std::string )> copyValues;

    Method<std::vector<int>()>       getIntVector;
    Method<void( std::vector<int> )> setIntVector;

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

    Field<std::string>           m_texts;
    ChildArrayField<DemoObject*> m_childArrayField;
};

class DemoDocument : public Document
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

    ChildField<DemoObject*>            demoObject;
    ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};

class DemoDocumentWithNonScriptableMember : public Document
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

    ChildField<DemoObject*>            demoObject;
    ChildField<DemoObject*>            demoObjectNonScriptable;
    ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};
