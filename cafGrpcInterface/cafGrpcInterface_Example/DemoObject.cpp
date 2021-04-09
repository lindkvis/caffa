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
    initObject( "Demo Object", "", "", "" );

    CAF_InitScriptableFieldNoDefault( &m_doubleVector, "doubleVector", "", "", "", "" );
    CAF_InitScriptableFieldNoDefault( &m_floatVector, "floatVector", "", "", "", "" );
    CAF_InitScriptableFieldNoDefault( &m_intVector, "intVector", "", "", "", "" );
}

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

InheritedDemoObj::InheritedDemoObj()
{
    initObject( "Inherited Demo Object", "", "", "" );
    this->addField( &m_texts, "Texts" );
    this->addField( &m_childArrayField, "DemoObjectects" );
    this->addField( &m_ptrField, "m_ptrField" );
}

CAF_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject" );

DemoDocument::DemoDocument()
{
    initObject( "DemoDocument", "", "Demo Document", "" );
    CAF_InitScriptableFieldNoDefault( &m_demoObject, "DemoObject", "", "", "", "" );
    CAF_InitScriptableFieldNoDefault( &m_inheritedDemoObjects, "InheritedDemoObject", "", "", "", "" );
    m_demoObject = new DemoObject;

    this->fileName = "dummyFileName";
}

CAF_SOURCE_INIT( DemoDocument, "DemoDocument" );
