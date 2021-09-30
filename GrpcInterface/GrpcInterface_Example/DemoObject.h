//##################################################################################################
//
//   Caffa
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
#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafObjectMethod.h"
#include "cafValueField.h"

#include <iostream>
#include <string>

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    DemoObject();

    ~DemoObject() override {}

    caffa::Field<double>      m_memberDoubleField;
    caffa::Field<int>         m_memberIntField;
    caffa::Field<std::string> m_memberStringField;

    caffa::Field<std::vector<double>> m_doubleVector;
    caffa::Field<std::vector<float>>  m_floatVector;
    caffa::Field<std::vector<int>>    m_intVector;

    double doubleMember() const { return m_memberDoubleField; }
    void   setDoubleMember( const double& d ) { m_memberDoubleField = d; }

    int  intMember() const { return m_memberIntField; }
    void setIntMember( const int& val ) { m_memberIntField = val; }

    std::string stringMember() const { return m_memberStringField; }
    void        setStringMember( const std::string& val ) { m_memberStringField = val; }

    std::vector<double> doubleVector() const { return m_doubleVector; }
    void                setDoubleVector( const std::vector<double>& values ) { m_doubleVector = values; }

    std::vector<float> floatVector() const { return m_floatVector; }
    void               setFloatVector( const std::vector<float>& values ) { m_floatVector = values; }

    std::vector<int> intVector() const { return m_intVector; }
    void             setIntVector( const std::vector<int>& values ) { m_intVector = values; }
};

struct DemoObject_copyObjectResult : public caffa::Object
{
    CAFFA_HEADER_INIT;

    DemoObject_copyObjectResult();

    caffa::Field<bool> status;
};

class DemoObject_copyObject : public caffa::ObjectMethod
{
    CAFFA_HEADER_INIT;

public:
    DemoObject_copyObject( caffa::ObjectHandle* self,
                           double               doubleValue = -123.0,
                           int                  intValue    = 42,
                           const std::string&   stringValue = "SomeValue" );
    std::pair<bool, std::unique_ptr<caffa::ObjectHandle>> execute() override;
    std::unique_ptr<ObjectHandle>                         defaultResult() const override;

private:
    caffa::Field<double>      m_memberDoubleField;
    caffa::Field<int>         m_memberIntField;
    caffa::Field<std::string> m_memberStringField;
};

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT;

public:
    InheritedDemoObj();

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};

class DemoDocument : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    DemoDocument();
    ~DemoDocument() override {}

    void addInheritedObject( std::unique_ptr<InheritedDemoObj> object )
    {
        m_inheritedDemoObjects.push_back( std::move( object ) );
    }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.value(); }

    DemoObject* demoObject() { return m_demoObject; }

private:
    caffa::ChildField<DemoObject*>            m_demoObject;
    caffa::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};
