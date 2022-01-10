#include "DemoObject.h"

#include "cafFieldProxyAccessor.h"

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" );

DemoObject::DemoObject()
    : m_proxyDoubleValue( 2.1 )
    , m_proxyIntValue( 7 )
    , m_proxyStringValue( "abba" )
{
    initField( m_proxyDoubleField, "proxyDoubleField" ).withScripting();
    initField( m_proxyIntField, "proxyIntField" ).withScripting();
    initField( m_proxyStringField, "proxyStringField" ).withScripting();

    auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
    doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleProxy );
    doubleProxyAccessor->registerGetMethod( this, &DemoObject::getDoubleProxy );
    m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

    auto intProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<int>>();
    intProxyAccessor->registerSetMethod( this, &DemoObject::setIntProxy );
    intProxyAccessor->registerGetMethod( this, &DemoObject::getIntProxy );
    m_proxyIntField.setAccessor( std::move( intProxyAccessor ) );

    auto stringProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::string>>();
    stringProxyAccessor->registerSetMethod( this, &DemoObject::setStringProxy );
    stringProxyAccessor->registerGetMethod( this, &DemoObject::getStringProxy );
    m_proxyStringField.setAccessor( std::move( stringProxyAccessor ) );

    initField( doubleVector, "doubleVector" ).withScripting();
    initField( floatVector, "floatVector" ).withScripting();
    initField( intVector, "proxyIntVector" ).withScripting();
    initField( stringVector, "proxyStringVector" ).withScripting();

    auto intVectorProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::vector<int>>>();
    intVectorProxyAccessor->registerSetMethod( this, &DemoObject::setIntVectorProxy );
    intVectorProxyAccessor->registerGetMethod( this, &DemoObject::getIntVectorProxy );
    intVector.setAccessor( std::move( intVectorProxyAccessor ) );

    auto stringVectorProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<std::vector<std::string>>>();
    stringVectorProxyAccessor->registerSetMethod( this, &DemoObject::setStringVectorProxy );
    stringVectorProxyAccessor->registerGetMethod( this, &DemoObject::getStringVectorProxy );
    stringVector.setAccessor( std::move( stringVectorProxyAccessor ) );

    initField( doubleField, "doubleField" ).withScripting().withDefault( 0.0 );
    initField( intField, "intField" ).withScripting().withDefault( 0 );
    initField( stringField, "stringField" ).withScripting().withDefault( "" );
    initField( intFieldNonScriptable, "intFieldNonScriptable" ).withDefault( -1 );

    initField( boolField, "boolField" ).withScripting();
    initField( boolVector, "boolVector" ).withScripting();
}

CAFFA_SOURCE_INIT( DemoObject_copyObjectResult, "DemoObject_copyObjectResult", "Object" );

CAFFA_OBJECT_METHOD_SOURCE_INIT( DemoObject, DemoObject_copyObject, "copyObject" );

CAFFA_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObject", "DemoObject" );

CAFFA_SOURCE_INIT( DemoDocument, "DemoDocument", "Document", "Object" );

CAFFA_SOURCE_INIT( DemoDocumentWithNonScriptableMember, "DemoDocumentNonScriptableMember", "Document", "Object" );
