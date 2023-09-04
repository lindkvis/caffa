#include "gtest.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldValidator.h"
#include "cafJsonSerializer.h"
#include "cafMethod.h"
#include "cafObject.h"

using namespace caffa;

class ChildObject : public Object
{
    // Repeat the class name and parent Caffa Object
    // This registers methods for inspecting the class hierarchy
    CAFFA_HEADER_INIT( ChildObject, Object )

public:
    // Caffa classes must be default instantiable, since they are created by a factory for serialization
    // But as long as they have default values for all parameters, we are good!
    ChildObject( const std::string& childName = "" );
    ~ChildObject() noexcept override = default;

public:
    Field<std::string> name;
};

class TinyDemoDocument : public Document
{
    CAFFA_HEADER_INIT_WITH_DOC( "A tiny object with documentation", TinyDemoDocument, Document )

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    TinyDemoDocument();
    ~TinyDemoDocument() noexcept override = default;

public:
    Field<bool>                   toggleField;
    Field<double>                 doubleField;
    Field<int>                    intField;
    Field<std::vector<int>>       intVectorField;
    Field<AppEnum<TestEnumType>>  enumField;
    ChildArrayField<ChildObject*> children; // ChildArrayFields hold a vector of Caffa Objects
    ChildField<ChildObject*>      specialChild; // Child fields hold a single Caffa Object

public:
    Method<void( double )> scaleDoubleField; // A registered method
};

CAFFA_SOURCE_INIT( ChildObject )

ChildObject::ChildObject( const std::string& childName )
{
    initField( name, "name" ).withDefault( childName ).withScripting();
}

CAFFA_SOURCE_INIT( TinyDemoDocument )

// Must be in caffa namespace
namespace caffa
{
template <>
void AppEnum<TinyDemoDocument::TestEnumType>::setUp()
{
    // Register enum values with a corresponding text string
    addItem( TinyDemoDocument::T1, "T1" );
    addItem( TinyDemoDocument::T2, "T2" );
    addItem( TinyDemoDocument::T3, "T3" );
    setDefault( TinyDemoDocument::T1 );
}

} // namespace caffa

TinyDemoDocument::TinyDemoDocument()
{
    initField( toggleField, "Toggle" ).withDefault( true ).withScripting();
    initField( doubleField, "Number" ).withDefault( 11.0 ).withScripting();
    initField( intField, "Integer" ).withDefault( 42 ).withScripting();
    initField( enumField, "Enum" ).withScripting();
    initField( intVectorField, "Integers" ).withScripting();
    initField( children, "Children" ).withScripting();
    initField( specialChild, "SpecialChild" ); // Omitted withScripting => not remote accessible.

    initMethod( scaleDoubleField,
                "scaleDouble",
                { "scalingFactor" },
                [this]( double scalingFactor )
                { this->doubleField.setValue( this->doubleField.value() * scalingFactor ); } );

    // Add a couple of children to the child array field
    children.push_back( std::make_shared<ChildObject>( "Alice" ) );
    children.push_back( std::make_shared<ChildObject>( "Bob" ) );

    // Set the single child object
    specialChild = std::make_shared<ChildObject>( "Balthazar" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReadmeObjectTest, ServerCreation )
{
    auto doc = std::make_shared<TinyDemoDocument>();

    try
    {
        doc->toggleField    = true;
        int currentIntValue = doc->intField;
        doc->scaleDoubleField( 3.0 );

        CAFFA_DEBUG( "Demo Doc: " << caffa::JsonSerializer().setSerializeUuids( false ).writeObjectToString( doc.get() ) );

        auto object = doc->children.objects().front();
        CAFFA_DEBUG(
            "Child Object: " << caffa::JsonSerializer().setSerializeUuids( false ).writeObjectToString( object.get() ) );
    }
    catch ( const std::exception& e )
    {
        FAIL();
    }
}