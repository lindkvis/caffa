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

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPdmDocument.h"
#include "cafPdmReferenceHelper.h"
#include "cafPtrField.h"
#include "cafValueField.h"

#include <iostream>
#include <string>

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject();

    ~DemoObject() override {}

    caf::Field<double>      m_memberDoubleField;
    caf::Field<int>         m_memberIntField;
    caf::Field<std::string> m_memberStringField;

    caf::Field<std::vector<double>> m_doubleVector;
    caf::Field<std::vector<float>>  m_floatVector;
    caf::Field<std::vector<int>>    m_intVector;

    std::vector<double> getDoubleVector() const { return m_doubleVector; }
    void                setDoubleVector( const std::vector<double>& values ) { m_doubleVector = values; }

    std::vector<float> getFloatVector() const { return m_floatVector; }
    void               setFloatVector( const std::vector<float>& values ) { m_floatVector = values; }

    std::vector<int> getIntVector() const { return m_intVector; }
    void             setIntVector( const std::vector<int>& values ) { m_intVector = values; }
};

class InheritedDemoObj : public DemoObject
{
    CAF_HEADER_INIT;

public:
    InheritedDemoObj();

    caf::Field<std::string>           m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
    caf::PtrField<InheritedDemoObj*>  m_ptrField;
};

class DemoDocument : public caf::PdmDocument
{
    CAF_HEADER_INIT;

public:
    DemoDocument();
    ~DemoDocument() override {}

    void addInheritedObject( InheritedDemoObj* object ) { m_inheritedDemoObjects.push_back( object ); }
    std::vector<InheritedDemoObj*> inheritedObjects() const { return m_inheritedDemoObjects.childObjects(); }

    caf::ChildField<DemoObject*>            m_demoObject;
    caf::ChildArrayField<InheritedDemoObj*> m_inheritedDemoObjects;
};
