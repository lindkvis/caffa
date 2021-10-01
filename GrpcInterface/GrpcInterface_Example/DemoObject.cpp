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
    initField( m_memberDoubleField, "doubleMember" ).withScripting().withDefault( 11.0 );
    initField( m_memberIntField, "intMember" ).withScripting();
    initField( m_memberStringField, "stringMember" ).withScripting();

    initField( m_doubleVector, "doubleVector" ).withScripting();
    initField( m_floatVector, "floatVector" ).withScripting();
    initField( m_intVector, "intVector" ).withScripting();
}

DemoObject_copyObjectResult::DemoObject_copyObjectResult()
{
    initField( status, "status" ).withDefault( false );
}

DemoObject_copyObject::DemoObject_copyObject( caffa::ObjectHandle* self,
                                              double               doubleValue,
                                              int                  intValue,
                                              const std::string&   stringValue )
    : caffa::ObjectMethod( self )
{
    initField( m_memberDoubleField, "doubleArgument" ).withScripting().withDefault( doubleValue );
    initField( m_memberIntField, "intArgument" ).withScripting().withDefault( intValue );
    initField( m_memberStringField, "stringArgument" ).withScripting().withDefault( stringValue );
}
std::pair<bool, std::unique_ptr<caffa::ObjectHandle>> DemoObject_copyObject::execute()
{
    CAFFA_DEBUG( "Executing object method on server with values: " << m_memberDoubleField() << ", " << m_memberIntField()
                                                                   << ", " << m_memberStringField() );
    auto demoObject = self<DemoObject>();
    demoObject->setDoubleMember( m_memberDoubleField );
    demoObject->setIntMember( m_memberIntField );
    demoObject->setStringMember( m_memberStringField );

    auto demoObjectResult    = std::make_unique<DemoObject_copyObjectResult>();
    demoObjectResult->status = true;
    return std::make_pair( true, std::move( demoObjectResult ) );
}
std::unique_ptr<caffa::ObjectHandle> DemoObject_copyObject::defaultResult() const
{
    return std::make_unique<DemoObject_copyObjectResult>();
}

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" );
CAFFA_SOURCE_INIT( DemoObject_copyObjectResult, "DemoObject_copyObjectResult", "Object" );
CAFFA_OBJECT_METHOD_SOURCE_INIT( DemoObject, DemoObject_copyObject, "DemoObject_copyObject" );

InheritedDemoObj::InheritedDemoObj()
{
    initField( m_texts, "Texts" ).withScripting();
    initField( m_childArrayField, "DemoObjects" ).withScripting();
}

CAFFA_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject", "DemoObject" );

DemoDocument::DemoDocument()
{
    initField( m_demoObject, "DemoObject" ).withScripting();
    initField( m_inheritedDemoObjects, "InheritedDemoObjects" ).withScripting();
    m_demoObject = std::make_unique<DemoObject>();

    this->setFileName( "dummyFileName" );
}

CAFFA_SOURCE_INIT( DemoDocument, "DemoDocument", "Document" );
