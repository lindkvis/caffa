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
#include "DemoObject.h"

#include "cafFieldScriptingCapability.h"

DemoObject::DemoObject()
{
    initObject();

    initField( m_doubleVector, "doubleVector" ).withScripting();
    initField( m_floatVector, "floatVector" ).withScripting();
    initField( m_intVector, "intVector" ).withScripting();
}

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

InheritedDemoObj::InheritedDemoObj()
{
    initObject();

    initField( m_texts, "Texts" ).withScripting();
    initField( m_childArrayField, "DemoObjects" ).withScripting();
    initField( m_ptrField, "m_ptrField" ).withScripting();
}

CAF_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject" );

DemoDocument::DemoDocument()
{
    initObject();

    initField( m_demoObject, "DemoObject" ).withScripting();
    initField( m_inheritedDemoObjects, "InheritedDemoObjects" ).withScripting();
    m_demoObject = new DemoObject;

    this->fileName = "dummyFileName";
}

CAF_SOURCE_INIT( DemoDocument, "DemoDocument" );
