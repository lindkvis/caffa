
#include "gtest/gtest.h"

#include "cafField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafJsonDataType.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"

#include <optional>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class ObjectWithOptionals : public caffa::Object
{
    CAFFA_HEADER_INIT( ObjectWithOptionals, Object )

public:
    enum TestEnumType
    {
        T1,
        T2
    };

    ObjectWithOptionals()
    {
        initField( m_number, "Number" );
        initField( m_flag, "Flag" );
        initField( m_text, "Text" );
        initField( m_enum, "Enum" );
    }

    caffa::Field<std::optional<double>>                       m_number;
    caffa::Field<std::optional<bool>>                         m_flag;
    caffa::Field<std::optional<std::string>>                  m_text;
    caffa::Field<std::optional<caffa::AppEnum<TestEnumType>>> m_enum;
};
CAFFA_SOURCE_INIT( ObjectWithOptionals )

namespace caffa
{
template <>
void AppEnum<ObjectWithOptionals::TestEnumType>::setUp()
{
    addItem( ObjectWithOptionals::T1, "T1" );
    addItem( ObjectWithOptionals::T2, "T2" );
    setDefault( ObjectWithOptionals::T1 );
}

} // namespace caffa

//--------------------------------------------------------------------------------------------------
/// An absent key leaves the field empty, while a stated value - including a false - is readable as
/// such. This is the distinction optional fields exist to express.
//--------------------------------------------------------------------------------------------------
TEST( SerializeOptionals, AbsentIsDistinctFromStated )
{
    ObjectWithOptionals object;

    ASSERT_FALSE( object.m_number.value().has_value() );
    ASSERT_FALSE( object.m_flag.value().has_value() );

    caffa::JsonSerializer().readObjectFromString( &object,
                                                  "{\"class\": \"ObjectWithOptionals\", \"Number\": 10.0, "
                                                  "\"Flag\": false}" );

    ASSERT_TRUE( object.m_number.value().has_value() );
    ASSERT_DOUBLE_EQ( 10.0, *object.m_number.value() );

    ASSERT_TRUE( object.m_flag.value().has_value() );
    ASSERT_FALSE( *object.m_flag.value() );

    ASSERT_FALSE( object.m_text.value().has_value() );
}

//--------------------------------------------------------------------------------------------------
/// An explicit null is skipped by the serializer, so it reads as absent rather than as a stated
/// empty value. Data models which need those to differ cannot use a single optional to do it.
//--------------------------------------------------------------------------------------------------
TEST( SerializeOptionals, ExplicitNullReadsAsAbsent )
{
    ObjectWithOptionals object;
    object.m_number = 10.0;

    caffa::JsonSerializer().readObjectFromString( &object, "{\"class\": \"ObjectWithOptionals\", \"Number\": null}" );

    ASSERT_TRUE( object.m_number.value().has_value() );
    ASSERT_DOUBLE_EQ( 10.0, *object.m_number.value() );
}

//--------------------------------------------------------------------------------------------------
/// An empty optional is left out of the written object entirely.
//--------------------------------------------------------------------------------------------------
TEST( SerializeOptionals, EmptyValuesAreNotWritten )
{
    ObjectWithOptionals object;
    object.m_flag = true;

    const auto json = caffa::JsonSerializer().writeObjectToString( &object );

    ASSERT_NE( std::string::npos, json.find( "\"Flag\":true" ) );
    ASSERT_EQ( std::string::npos, json.find( "\"Number\"" ) );
}

//--------------------------------------------------------------------------------------------------
/// Schemas are JSON Schema 2020-12, which spells nullability as a type union, and as an anyOf
/// where there is no plain type to make a union of.
//--------------------------------------------------------------------------------------------------
TEST( SerializeOptionals, Schema )
{
    const auto numberType = caffa::JsonDataType<std::optional<double>>::jsonType();
    ASSERT_EQ( "[\"number\",\"null\"]", caffa::json::dump( numberType.at( "type" ) ) );
    ASSERT_EQ( "\"double\"", caffa::json::dump( numberType.at( "format" ) ) );

    // Wrapping an optional in another optional must widen the existing union rather than nest it.
    const auto nestedType = caffa::JsonDataType<std::optional<std::optional<double>>>::jsonType();
    ASSERT_EQ( "[\"number\",\"null\"]", caffa::json::dump( nestedType.at( "type" ) ) );

    const auto enumType =
        caffa::JsonDataType<std::optional<caffa::AppEnum<ObjectWithOptionals::TestEnumType>>>::jsonType();
    ASSERT_FALSE( enumType.contains( "type" ) );
    ASSERT_EQ( 2u, enumType.at( "anyOf" ).as_array().size() );
    ASSERT_EQ( "{\"type\":\"null\"}", caffa::json::dump( enumType.at( "anyOf" ).as_array().at( 1 ) ) );
}
