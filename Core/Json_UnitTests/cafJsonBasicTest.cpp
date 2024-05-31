
#include "gtest.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldRangeValidator.h"
#include "cafJsonSerializer.h"
#include "cafMethod.h"
#include "cafObject.h"
#include "cafStringEncoding.h"

#include <functional>
#include <random>

using namespace std::placeholders;

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT( DemoObject, Object )

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    DemoObject()
    {
        initField( m_proxyDoubleField, "BigNumber" );

        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( std::bind( &DemoObject::setDoubleMember, this, _1 ) );
        doubleProxyAccessor->registerGetMethod( std::bind( &DemoObject::doubleMember, this ) );
        m_proxyDoubleField.setAccessor( std::move( doubleProxyAccessor ) );

        initField( m_proxyEnumField, "EnumField" );
        auto proxyEnumAccessor = std::make_unique<caffa::FieldProxyAccessor<caffa::AppEnum<TestEnumType>>>();
        proxyEnumAccessor->registerSetMethod( std::bind( &DemoObject::setEnumMember, this, _1 ) );
        proxyEnumAccessor->registerGetMethod( std::bind( &DemoObject::enumMember, this ) );
        m_proxyEnumField.setAccessor( std::move( proxyEnumAccessor ) );

        m_enumMember = T1;

        initMethod( getEnum,
                    "getEnum",
                    [this]() -> caffa::AppEnum<TestEnumType> { return this->m_proxyEnumField.value(); } )
            .makeConst();

        initMethod( setEnum,
                    "setEnum",
                    [this]( caffa::AppEnum<TestEnumType> val ) -> void { return m_proxyEnumField.setValue( val ); } )
            .withArgumentNames( { "enum" } );
        initMethod( clone,
                    "clone",
                    std::bind(
                        [this]() -> std::shared_ptr<DemoObject>
                        {
                            caffa::ObjectFactory* objectFactory = caffa::DefaultObjectFactory::instance();
                            auto                  object =
                                caffa::JsonSerializer( objectFactory ).setSerializeUuids( false ).copyBySerialization( this );
                            return std::dynamic_pointer_cast<DemoObject>( object );
                        } ) )
            .makeConst();

        initMethod( copyFrom, "copyFrom", std::bind( &DemoObject::_copyFrom, this, std::placeholders::_1 ) )
            .withArgumentNames( { "rhs" } )
            .makeConst();
    }

    ~DemoObject() {}

    // Fields

    caffa::Field<caffa::AppEnum<TestEnumType>>               m_proxyEnumField;
    caffa::Field<double>                                     m_proxyDoubleField;
    caffa::Method<caffa::AppEnum<TestEnumType>()>            getEnum;
    caffa::Method<void( caffa::AppEnum<TestEnumType> )>      setEnum;
    caffa::Method<std::shared_ptr<DemoObject>()>             clone;
    caffa::Method<void( std::shared_ptr<const DemoObject> )> copyFrom;

private:
    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }

    void setEnumMember( const caffa::AppEnum<TestEnumType>& val ) { m_enumMember = val.value(); }
    caffa::AppEnum<TestEnumType> enumMember() const { return m_enumMember; }
    void                         _copyFrom( std::shared_ptr<const DemoObject> rhs )
    {
        caffa::JsonSerializer serializer;
        std::string           json = serializer.writeObjectToString( rhs.get() );
        serializer.readObjectFromString( this, json );
    }

    double       m_doubleMember;
    TestEnumType m_enumMember;
};

CAFFA_SOURCE_INIT( DemoObject )

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

TEST( BaseTest, Delete )
{
    DemoObject* s2 = new DemoObject;
    delete s2;
}

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, FieldWrite )
{
    std::string serializedString;
    {
        DemoObject a;

        a.m_proxyDoubleField.setValue( 2.5 );
        a.m_proxyEnumField = DemoObject::T3;

        ASSERT_DOUBLE_EQ( 2.5, a.m_proxyDoubleField.value() );
        ASSERT_EQ( DemoObject::T3, a.m_proxyEnumField.value() );

        serializedString = caffa::JsonSerializer().writeObjectToString( &a );

        std::cout << serializedString << std::endl;
    }

    std::string secondSerializedString;
    {
        auto a                 = caffa::JsonSerializer().createObjectFromString( serializedString );
        secondSerializedString = caffa::JsonSerializer().writeObjectToString( a.get() );
    }
    ASSERT_EQ( serializedString, secondSerializedString );
}

TEST( BaseTest, Methods )
{
    DemoObject object;

    EXPECT_EQ( DemoObject::T1, object.getEnum() );
    object.setEnum( DemoObject::T2 );

    object.m_proxyDoubleField = 589.123;

    EXPECT_DOUBLE_EQ( 589.123, object.m_proxyDoubleField() );

    auto result = object.clone();
    EXPECT_TRUE( result != nullptr );
    EXPECT_DOUBLE_EQ( object.m_proxyDoubleField(), result->m_proxyDoubleField() );

    caffa::JsonSerializer serializer;
    CAFFA_DEBUG( "Original: " << serializer.writeObjectToString( &object ) );
    CAFFA_DEBUG( "Clone: " << serializer.writeObjectToString( result.get() ) );
}

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, MethodWrite )
{
    std::string serializedString;
    {
        DemoObject a;

        a.setEnum( DemoObject::T3 );

        ASSERT_EQ( DemoObject::T3, a.getEnum() );

        serializedString = caffa::JsonSerializer().writeObjectToString( &a );

        std::cout << serializedString << std::endl;
    }

    std::string secondSerializedString;
    {
        auto a                 = caffa::JsonSerializer().createObjectFromString( serializedString );
        secondSerializedString = caffa::JsonSerializer().writeObjectToString( a.get() );
    }
    ASSERT_EQ( serializedString, secondSerializedString );
}

class InheritedDemoObj : public DemoObject
{
    CAFFA_HEADER_INIT( InheritedDemoObj, DemoObject )

public:
    InheritedDemoObj()
    {
        initField( m_texts, "Texts" );
        initField( m_childArrayField, "DemoObjects" );
    }

    caffa::Field<std::string>           m_texts;
    caffa::ChildArrayField<DemoObject*> m_childArrayField;
};
CAFFA_SOURCE_INIT( InheritedDemoObj )

using IntFieldRangeValidator = caffa::FieldRangeValidator<int>;

class SimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT( SimpleObj, Object )

public:
    SimpleObj()
        : m_doubleMember( 0.0 )
    {
        initField( m_position, "Position" );
        initField( m_dir, "Dir" );
        initField( m_up, "Up" );

        initField( m_proxyDouble, "m_proxyDouble" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( std::bind( &SimpleObj::setDoubleMember, this, _1 ) );
        doubleProxyAccessor->registerGetMethod( std::bind( &SimpleObj::doubleMember, this ) );
        m_proxyDouble.setAccessor( std::move( doubleProxyAccessor ) );
    }

    caffa::Field<double> m_position;
    caffa::Field<double> m_dir;
    caffa::Field<int>    m_up;
    caffa::Field<double> m_proxyDouble;

    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }
    void setUpRange( int minimum, int maximum )
    {
        m_up.addValidator( std::make_unique<IntFieldRangeValidator>( minimum, maximum ) );
    }

    double m_doubleMember;
};
CAFFA_SOURCE_INIT( SimpleObj )

class ReferenceDemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT( ReferenceDemoObject, Object )

public:
    ReferenceDemoObject()
    {
        initField( m_pointersField, "SimpleObjPtrField" );
        initField( m_simpleObjPtrField2, "SimpleObjPtrField2" );
    }

    // Fields
    caffa::ChildField<ObjectHandle*>   m_pointersField;
    caffa::ChildArrayField<SimpleObj*> m_simpleObjPtrField2;
};

CAFFA_SOURCE_INIT( ReferenceDemoObject )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldSerializing )
{
    auto s1 = std::make_shared<DemoObject>();
    auto s2 = std::make_shared<DemoObject>();
    auto s3 = std::make_shared<DemoObject>();

    s1->m_proxyDoubleField.setValue( 10 );
    s2->m_proxyDoubleField.setValue( 20 );
    s3->m_proxyDoubleField.setValue( 30 );

    std::string serializedString;
    {
        auto ihd1 = std::make_shared<InheritedDemoObj>();
        ihd1->m_childArrayField.push_back( s1 );
        ihd1->m_childArrayField.push_back( s2 );
        ihd1->m_childArrayField.push_back( s3 );

        serializedString = caffa::JsonSerializer().writeObjectToString( ihd1.get() );

        std::cout << "Wrote object to json: " << serializedString << std::endl;
    }

    {
        auto ihd1 = std::make_shared<InheritedDemoObj>();
        ASSERT_EQ( 0u, ihd1->m_childArrayField.size() );

        caffa::JsonSerializer().readObjectFromString( ihd1.get(), serializedString );
        ASSERT_EQ( 3u, ihd1->m_childArrayField.size() );

        ASSERT_TRUE( ihd1->m_childArrayField[0] != nullptr );
        ASSERT_TRUE( ihd1->m_childArrayField[1] != nullptr );
        ASSERT_TRUE( ihd1->m_childArrayField[2] != nullptr );
        ASSERT_DOUBLE_EQ( 10, ihd1->m_childArrayField[0]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 20, ihd1->m_childArrayField[1]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 30, ihd1->m_childArrayField[2]->m_proxyDoubleField.value() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestDataType )
{
    auto s1 = std::make_shared<SimpleObj>();

    {
        auto dataType = s1->m_position.dataType();
        EXPECT_EQ( "double", dataType );
    }

    {
        auto dataType = s1->m_proxyDouble.dataType();
        EXPECT_EQ( "double", dataType );
    }

    {
        auto dataType = s1->m_up.dataType();
        EXPECT_EQ( "int32", dataType );
    }

    {
        auto obj      = std::make_shared<InheritedDemoObj>();
        auto dataType = obj->m_childArrayField.dataType();

        // We've stored an InheritedDemoObj in the field, but the field is actually of the parent type DemoObject
        EXPECT_EQ( ( std::string( "object::" ) + DemoObject::classKeywordStatic() + "[]" ), dataType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestRangeValidation )
{
    auto s1 = std::make_shared<SimpleObj>();
    ASSERT_NO_THROW( s1->m_up.setValue( -10 ) );
    ASSERT_NO_THROW( s1->m_up.setValue( 0 ) );
    s1->setUpRange( 0, 10 );
    ASSERT_NO_THROW( s1->m_up.setValue( 5 ) );
    ASSERT_THROW( s1->m_up.setValue( 12 ), std::runtime_error );
    ASSERT_THROW( s1->m_up.setValue( -2 ), std::runtime_error );
    ASSERT_EQ( 5, s1->m_up );
}

std::string ipsum()
{
    return "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed aliquam ligula sed nibh rutrum, quis tempus "
           "neque finibus. Sed ante elit, facilisis ut libero ut, elementum congue nibh. Cras aliquam tellus eu cursus "
           "molestie. Aenean ut felis ut mi convallis dictum. Sed laoreet nisi in varius mattis. Integer eget arcu "
           "ligula. Cras in cursus urna. Aenean cursus bibendum efficitur. Suspendisse vel ex mauris. Nulla facilisi. "
           "Nulla vel ipsum imperdiet, volutpat dui interdum, ultrices erat. Aliquam ac bibendum erat. Proin elit sem, "
           "mollis vel tortor eget, vulputate posuere eros. Fusce fringilla a eros nec imperdiet. Quisque viverra "
           "tempor "
           "turpis, a ornare eros imperdiet sit amet.";
}

TEST( Base64Test, encode64ipsum )
{
    std::string expected =
        "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4gU2VkIGFsaXF1YW0gbGlndWxhIHNlZCBuaW"
        "JoIHJ1dHJ1bSwgcXVpcyB0ZW1wdXMgbmVxdWUgZmluaWJ1cy4gU2VkIGFudGUgZWxpdCwgZmFjaWxpc2lzIHV0IGxpYmVybyB1dCwgZWxlbWVu"
        "dHVtIGNvbmd1ZSBuaWJoLiBDcmFzIGFsaXF1YW0gdGVsbHVzIGV1IGN1cnN1cyBtb2xlc3RpZS4gQWVuZWFuIHV0IGZlbGlzIHV0IG1pIGNvbn"
        "ZhbGxpcyBkaWN0dW0uIFNlZCBsYW9yZWV0IG5pc2kgaW4gdmFyaXVzIG1hdHRpcy4gSW50ZWdlciBlZ2V0IGFyY3UgbGlndWxhLiBDcmFzIGlu"
        "IGN1cnN1cyB1cm5hLiBBZW5lYW4gY3Vyc3VzIGJpYmVuZHVtIGVmZmljaXR1ci4gU3VzcGVuZGlzc2UgdmVsIGV4IG1hdXJpcy4gTnVsbGEgZm"
        "FjaWxpc2kuIE51bGxhIHZlbCBpcHN1bSBpbXBlcmRpZXQsIHZvbHV0cGF0IGR1aSBpbnRlcmR1bSwgdWx0cmljZXMgZXJhdC4gQWxpcXVhbSBh"
        "YyBiaWJlbmR1bSBlcmF0LiBQcm9pbiBlbGl0IHNlbSwgbW9sbGlzIHZlbCB0b3J0b3IgZWdldCwgdnVscHV0YXRlIHBvc3VlcmUgZXJvcy4gRn"
        "VzY2UgZnJpbmdpbGxhIGEgZXJvcyBuZWMgaW1wZXJkaWV0LiBRdWlzcXVlIHZpdmVycmEgdGVtcG9yIHR1cnBpcywgYSBvcm5hcmUgZXJvcyBp"
        "bXBlcmRpZXQgc2l0IGFtZXQu";

    std::string encoded = caffa::StringTools::encodeBase64( ipsum() );

    ASSERT_EQ( expected, encoded );
}

TEST( Base64Test, decode64ipsum )
{
    std::string encoded =
        "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4gU2VkIGFsaXF1YW0gbGlndWxhIHNlZCBuaW"
        "JoIHJ1dHJ1bSwgcXVpcyB0ZW1wdXMgbmVxdWUgZmluaWJ1cy4gU2VkIGFudGUgZWxpdCwgZmFjaWxpc2lzIHV0IGxpYmVybyB1dCwgZWxlbWVu"
        "dHVtIGNvbmd1ZSBuaWJoLiBDcmFzIGFsaXF1YW0gdGVsbHVzIGV1IGN1cnN1cyBtb2xlc3RpZS4gQWVuZWFuIHV0IGZlbGlzIHV0IG1pIGNvbn"
        "ZhbGxpcyBkaWN0dW0uIFNlZCBsYW9yZWV0IG5pc2kgaW4gdmFyaXVzIG1hdHRpcy4gSW50ZWdlciBlZ2V0IGFyY3UgbGlndWxhLiBDcmFzIGlu"
        "IGN1cnN1cyB1cm5hLiBBZW5lYW4gY3Vyc3VzIGJpYmVuZHVtIGVmZmljaXR1ci4gU3VzcGVuZGlzc2UgdmVsIGV4IG1hdXJpcy4gTnVsbGEgZm"
        "FjaWxpc2kuIE51bGxhIHZlbCBpcHN1bSBpbXBlcmRpZXQsIHZvbHV0cGF0IGR1aSBpbnRlcmR1bSwgdWx0cmljZXMgZXJhdC4gQWxpcXVhbSBh"
        "YyBiaWJlbmR1bSBlcmF0LiBQcm9pbiBlbGl0IHNlbSwgbW9sbGlzIHZlbCB0b3J0b3IgZWdldCwgdnVscHV0YXRlIHBvc3VlcmUgZXJvcy4gRn"
        "VzY2UgZnJpbmdpbGxhIGEgZXJvcyBuZWMgaW1wZXJkaWV0LiBRdWlzcXVlIHZpdmVycmEgdGVtcG9yIHR1cnBpcywgYSBvcm5hcmUgZXJvcyBp"
        "bXBlcmRpZXQgc2l0IGFtZXQu";

    std::string decoded = caffa::StringTools::decodeBase64( encoded );
    ASSERT_EQ( ipsum(), decoded );
}

class Base64RoundTrip
{
public:
    Base64RoundTrip()
    {
        constexpr size_t maxLength = 5000;
        encodedStrings.reserve( maxLength );
        for ( size_t len = 1; len <= maxLength; ++len )
        {
            auto rs = randomString( len );
            encodedStrings.push_back( rs );
        }
    }

    static std::string randomString( size_t len )
    {
        static std::random_device                 rd;
        static std::mt19937                       mt( rd() );
        static std::uniform_int_distribution<int> dist( 0, 25 );

        std::string string;
        string.reserve( len );

        for ( size_t i = 0; i < len; ++i )
        {
            string.push_back( 'a' + dist( mt ) );
        }
        return string;
    }

    std::vector<std::string> encodedStrings;
};

Base64RoundTrip roundTripHolder;
std::string     oneLongString = Base64RoundTrip::randomString( 50 * 1024 * 1024 );

TEST( Base64Test, roundtrip )
{
    CAFFA_INFO( "Got " << roundTripHolder.encodedStrings.size() << " strings to encode and decode" );
    for ( auto string : roundTripHolder.encodedStrings )
    {
        auto encoded = caffa::StringTools::encodeBase64( string );
        auto decoded = caffa::StringTools::decodeBase64( encoded );
        ASSERT_EQ( string, decoded );
    }
}

TEST( Base64Test, oneLongRoundtrip )
{
    CAFFA_INFO( "Got a string of size " << oneLongString.length() / 1024 / 1024 << " MebiBytes to encode and decode" );
    auto encoded = caffa::StringTools::encodeBase64( oneLongString );
    CAFFA_INFO( "Encoded string is " << encoded.length() / 1024 / 1024 << " MebiBytes" );
    auto decoded = caffa::StringTools::decodeBase64( encoded );
    ASSERT_EQ( oneLongString, decoded );
}