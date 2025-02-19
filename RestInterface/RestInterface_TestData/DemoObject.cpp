#include "DemoObject.h"

#include "cafFieldProxyAccessor.h"
#include "cafRangeValidator.h"

#include <functional>

CAFFA_SOURCE_INIT( DemoObject )

using namespace std::placeholders;
using namespace caffa;

namespace caffa
{
template <>
void AppEnum<DemoObject::TestEnumType>::setUp()
{
    addItem( DemoObject::T1, "T1" );
    addItem( DemoObject::T2, "T2" );
    addItem( DemoObject::T3, "T3" );
    setDefault( DemoObject::T1 );
}

} // namespace caffa

DemoObject::DemoObject()
    : m_proxyDoubleValue( 2.1 )
    , m_proxyIntValue( 7 )
    , m_proxyStringValue( "abba" )
{
    initField( m_proxyDoubleField, "proxyDoubleField" ).withScripting();
    initField( m_proxyIntField, "proxyIntField" ).withScripting();
    initField( m_proxyStringField, "proxyStringField" ).withScripting();

    auto doubleProxyAccessor = std::make_unique<FieldProxyAccessor<double>>();
    doubleProxyAccessor->registerSetMethod( std::bind( &DemoObject::setDoubleProxy, this, _1 ) );
    doubleProxyAccessor->registerGetMethod( std::bind( &DemoObject::getDoubleProxy, this ) );
    m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

    auto intProxyAccessor = std::make_unique<FieldProxyAccessor<int>>();
    intProxyAccessor->registerSetMethod( std::bind( &DemoObject::setIntProxy, this, _1 ) );
    intProxyAccessor->registerGetMethod( std::bind( &DemoObject::getIntProxy, this ) );
    m_proxyIntField.setAccessor( std::move( intProxyAccessor ) );

    auto stringProxyAccessor = std::make_unique<FieldProxyAccessor<std::string>>();
    stringProxyAccessor->registerSetMethod( std::bind( &DemoObject::setStringProxy, this, _1 ) );
    stringProxyAccessor->registerGetMethod( std::bind( &DemoObject::getStringProxy, this ) );
    m_proxyStringField.setAccessor( std::move( stringProxyAccessor ) );

    initField( doubleVector, "doubleVector" ).withScripting();
    initField( floatVector, "floatVector" ).withScripting();
    initField( intVector, "proxyIntVector" ).withScripting();
    initField( stringVector, "proxyStringVector" ).withScripting();
    initField( enumField, "enumField" ).withScripting();

    auto intVectorProxyAccessor = std::make_unique<FieldProxyAccessor<std::vector<int>>>();
    intVectorProxyAccessor->registerSetMethod( std::bind( &DemoObject::setIntVectorProxy, this, _1 ) );
    intVectorProxyAccessor->registerGetMethod( std::bind( &DemoObject::getIntVectorProxy, this ) );
    intVector.setAccessor( std::move( intVectorProxyAccessor ) );

    auto stringVectorProxyAccessor = std::make_unique<FieldProxyAccessor<std::vector<std::string>>>();
    stringVectorProxyAccessor->registerSetMethod( std::bind( &DemoObject::setStringVectorProxy, this, _1 ) );
    stringVectorProxyAccessor->registerGetMethod( std::bind( &DemoObject::getStringVectorProxy, this ) );
    stringVector.setAccessor( std::move( stringVectorProxyAccessor ) );

    initField( doubleField, "doubleField" ).withScripting().withDefault( 0.0 );
    initField( intField, "intField" ).withScripting().withDefault( 0 ).withValidator( RangeValidator<int>::create( -100, 2000 ) );
    initField( stringField, "stringField" ).withScripting().withDefault( "" );
    initField( intFieldNonScriptable, "intFieldNonScriptable" ).withDefault( -1 );

    initField( boolField, "boolField" ).withScripting();
    initField( boolVector, "boolVector" ).withScripting();
    initField( floatMap, "floatMap" ).withScripting();
    initField( stringMap, "stringMap" ).withScripting();
    initField( enumMap, "enumMap" ).withScripting();

    initMethod( copyValues,
                "copyValues",
                std::bind( &DemoObject::_copyValues, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) )
        .withArgumentNames( { "intValue", "doubleValue", "stringValue" } );

    initMethod( setIntVector, "setIntVector", std::bind( &DemoObject::setIntVectorProxy, this, std::placeholders::_1 ) )
        .withArgumentNames( { "intVector" } );

    initMethod( getIntVector, "getIntVector", std::bind( &DemoObject::getIntVectorProxy, this ) ).makeConst();
}

void DemoObject::_copyValues( int intValue, double doubleValue, std::string stringValue )
{
    this->intField    = intValue;
    this->doubleField = doubleValue;
    this->stringField = stringValue;
}

CAFFA_SOURCE_INIT( InheritedDemoObj )

CAFFA_SOURCE_INIT( DemoDocument )

CAFFA_SOURCE_INIT( DemoDocumentWithNonScriptableMember )

CAFFA_SOURCE_INIT( UnrelatedDemoObject )