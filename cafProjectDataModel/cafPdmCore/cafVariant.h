#pragma once

#include "cafAssert.h"
#include "cafColor.h"
#include "cafFactory.h"
#include "cafPdmPointer.h"
#include "cafTristate.h"

#include <algorithm>
#include <ctime>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

namespace caf
{
class ObjectHandle;

template <typename>
struct is_std_vector : std::false_type
{
};

template <typename T, typename A>
struct is_std_vector<std::vector<T, A>> : std::true_type
{
};

/**
 * The fallback serializable wrapper for storing any generic object with serialization operators
 */
class SerializableWrapper
{
public:
    SerializableWrapper( const std::vector<std::string>& content, std::size_t typeHashCode, bool isVector )
        : m_content( content )
        , m_typeHashCode( typeHashCode )
        , m_isVector( isVector )
    {
    }

    template <typename T>
    explicit SerializableWrapper( T value )
        : m_content( serialize<T>( value ) )
        , m_typeHashCode( typeid( T ).hash_code() )
        , m_isVector( is_std_vector<T>::value )
    {
    }

    template <typename T>
    bool canConvert() const
    {
        return m_typeHashCode == typeid( T ).hash_code();
    }

    template <typename T>
    T value() const
    {
        CAF_ASSERT( m_typeHashCode == typeid( T ).hash_code() );
        return deserialize<T>( m_content );
    }

    std::string textValue() const
    {
        std::string text;
        if ( m_isVector )
        {
            for ( size_t i = 0; i < m_content.size(); ++i )
            {
                if ( i > 0 ) text += ", ";
                text += m_content[i];
            }
        }
        else if ( !m_content.empty() )
        {
            text = m_content.front();
        }
        return text;
    }

    bool isValid() const
    {
        return m_typeHashCode != 0u &&
               std::any_of( m_content.begin(), m_content.end(), []( const std::string& str ) { return !str.empty(); } );
    }

    bool isVector() const { return m_isVector; }

    bool operator==( const SerializableWrapper& rhs ) const
    {
        return m_typeHashCode == rhs.m_typeHashCode && m_content == rhs.m_content;
    }

    bool operator<( const SerializableWrapper& rhs ) const
    {
        if ( m_typeHashCode != rhs.m_typeHashCode )
        {
            return m_typeHashCode < rhs.m_typeHashCode;
        }

        if ( m_isVector != rhs.m_isVector )
        {
            return m_isVector < rhs.m_isVector;
        }

        if ( m_content != rhs.m_content )
        {
            return m_content < rhs.m_content;
        }
        return false;
    }

    std::vector<SerializableWrapper> toVector() const
    {
        CAF_ASSERT( m_isVector );
        std::vector<SerializableWrapper> vectorOfWrappers;
        for ( const std::string& valueString : m_content )
        {
            vectorOfWrappers.push_back( SerializableWrapper( { valueString }, m_typeHashCode, false ) );
        }
        return vectorOfWrappers;
    }

private:
    template <typename T>
    static std::vector<std::string> serialize( T value )
    {
        if constexpr ( is_std_vector<T>::value )
        {
            std::vector<std::string> stringVector;
            for ( auto entry : value )
            {
                std::stringstream stream;
                stream << entry;
                stringVector.push_back( stream.str() );
            }
            return stringVector;
        }
        else
        {
            std::stringstream stream;
            stream << value;
            CAF_ASSERT( stream.str().length() > 0u );
            return { stream.str() };
        }
    }

    template <typename T>
    static T deserialize( const std::vector<std::string>& stringValues )
    {
        if constexpr ( is_std_vector<T>::value )
        {
            T valueVector;

            for ( auto stringValue : stringValues )
            {
                std::stringstream      stream( stringValue );
                typename T::value_type value;
                stream >> value;
                valueVector.push_back( value );
            }
            return valueVector;
        }
        else
        {
            CAF_ASSERT( stringValues.size() == 1u );
            std::stringstream stream( stringValues.front() );
            T                 value;
            stream >> value;
            return value;
        }
    }

private:
    std::vector<std::string> m_content;
    std::size_t              m_typeHashCode;
    bool                     m_isVector;
};

// Main lookup logic of looking up a type in a list.
template <typename T, typename... ALL_T>
struct isOneOf : public std::false_type
{
};

template <typename T, typename FRONT_T, typename... REST_T>
struct isOneOf<T, FRONT_T, REST_T...>
    : public std::conditional<std::is_same<T, FRONT_T>::value, std::true_type, isOneOf<T, REST_T...>>::type
{
};

// Convenience wrapper for std::variant<>.
template <typename T, typename VARIANT_T>
struct isVariantMember;

template <typename T, typename... ALL_T>
struct isVariantMember<T, std::variant<ALL_T...>> : public isOneOf<T, ALL_T...>
{
};

/**
 * A think wrapper around std::variant, mainly to avoid creating std::any of literal character strings
 */
class Variant
{
public:
    using InternalVariant = std::variant<bool,
                                         int,
                                         unsigned char,
                                         unsigned int,
                                         uint64_t,
                                         double,
                                         float,
                                         std::string,
                                         caf::Tristate,
                                         caf::Color,
                                         std::time_t,
                                         caf::PdmPointer<ObjectHandle>,
                                         std::vector<bool>,
                                         std::vector<int>,
                                         std::vector<unsigned char>,
                                         std::vector<unsigned int>,
                                         std::vector<uint64_t>,
                                         std::vector<double>,
                                         std::vector<float>,
                                         std::vector<std::string>,
                                         std::vector<caf::Tristate>,
                                         std::vector<caf::Color>,
                                         std::vector<std::time_t>,
                                         std::vector<caf::PdmPointer<ObjectHandle>>>;
    Variant() {}

    template <typename T>
    Variant( const T& value )
    {
        if constexpr ( !std::is_same<std::nullptr_t, T>::value )
        {
            if constexpr ( std::is_same<unsigned int, T>::value )
            {
                CAF_ASSERT( false );
            }
            else if constexpr ( isVariantMember<T, InternalVariant>::value )
            {
                m_data = value;
            }
            else
            {
                m_serializableData = SerializableWrapper( value );
            }
        }
    }

    template <std::size_t N>
    Variant( const char ( &value )[N] )
    {
        m_data = std::string( value );
    }

    Variant( const SerializableWrapper& wrapper )
        : m_serializableData( wrapper )
    {
    }

    Variant( const Variant& rhs );

    Variant& operator=( const Variant& rhs );

    template <typename T>
    bool canConvert() const
    {
        if constexpr ( isVariantMember<T, InternalVariant>::value )
        {
            return m_data.has_value() && std::holds_alternative<T>( m_data.value() );
        }
        else
        {
            return m_serializableData.has_value() && m_serializableData->canConvert<T>();
        }
    }

    template <typename T>
    T value() const
    {
        if constexpr ( isVariantMember<T, InternalVariant>::value )
        {
            if ( !m_data.has_value() ) throw std::bad_variant_access();
            return std::get<T>( *m_data );
        }
        else
        {
            CAF_ASSERT( m_serializableData.has_value() );
            return m_serializableData.value().value<T>();
        }
    }

    std::string textValue() const;

    template <typename T>
    T value( T defaultValue ) const noexcept
    {
        if constexpr ( isVariantMember<T, InternalVariant>::value )
        {
            if ( !m_data.has_value() || !std::holds_alternative<T>( m_data.value() ) ) return defaultValue;
            return std::get<T>( *m_data );
        }
        else
        {
            if ( !m_serializableData.has_value() ) return defaultValue;
            return m_serializableData.value().value<T>();
        }
    }

    bool operator==( const Variant& rhs ) const
    {
        bool hasValue    = m_data.has_value();
        bool rhsHasValue = rhs.m_data.has_value();
        if ( hasValue != rhsHasValue ) return false;
        if ( hasValue )
        {
            return m_data.value() == rhs.m_data.value();
        }
        else if ( m_serializableData && rhs.m_serializableData )
        {
            return m_serializableData == rhs.m_serializableData;
        }
        return false;
    }

    bool operator!=( const Variant& rhs ) const { return !( *this == rhs ); }

    // Set operator
    bool operator<( const Variant& rhs ) const;

    bool isValid() const;
    bool isVector() const;

    std::vector<Variant> toVector() const;
    static Variant       fromVector( const std::vector<Variant>& variantVector );

private:
    template <typename T>
    static std::vector<Variant> toVectorT( const Variant& variant );
    template <typename T>
    static Variant fromVectorT( const std::vector<Variant>& variantVector );

private:
    std::optional<InternalVariant>     m_data;
    std::optional<SerializableWrapper> m_serializableData;
};

} // namespace caf
