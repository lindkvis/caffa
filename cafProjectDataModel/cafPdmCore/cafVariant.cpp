#include "cafVariant.h"

#include "cafAssert.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Variant::isValid() const
{
    return m_data.has_value() || ( m_serializableData.has_value() && m_serializableData->isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Variant::Variant( const Variant& rhs )
{
    if ( rhs.m_data.has_value() )
    {
        m_data = rhs.m_data;
    }
    else if ( rhs.m_serializableData.has_value() )
    {
        m_serializableData = SerializableWrapper( rhs.m_serializableData.value() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Variant& Variant::operator=( const Variant& rhs )
{
    if ( rhs.m_data.has_value() )
    {
        m_data = rhs.m_data;
    }
    else if ( rhs.m_serializableData.has_value() )
    {
        m_serializableData = SerializableWrapper( rhs.m_serializableData.value() );
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Variant::isVector() const
{
    if ( m_data.has_value() )
    {
        return std::visit(
            []( auto&& arg ) -> bool {
                using T = std::decay_t<decltype( arg )>;
                if constexpr ( is_std_vector<T>::value )
                {
                    return true;
                }
                else
                {
                    return false;
                }
            },
            m_data.value() );
    }
    else if ( m_serializableData )
    {
        return m_serializableData->isVector();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Variant> Variant::toVector() const
{
    CAF_ASSERT( isVector() );
    if ( m_data.has_value() )
    {
        std::vector<Variant> v = std::visit(
            []( auto&& arg ) -> std::vector<Variant> {
                using T = std::decay_t<decltype( arg )>;
                if constexpr ( isVariantMember<T, InternalVariant>::value )
                {
                    if constexpr ( is_std_vector<T>::value )
                    {
                        return toVectorT<typename T::value_type>( arg );
                    }
                    else
                    {
                        return std::vector<Variant>();
                    }
                }
                else
                {
                    CAF_ASSERT( false );
                    return std::vector<Variant>();
                }
            },
            m_data.value() );
        return v;
    }
    else if ( m_serializableData.has_value() )
    {
        std::vector<Variant>             variantVector;
        std::vector<SerializableWrapper> serializableVector = m_serializableData->toVector();
        for ( const SerializableWrapper& wrapper : serializableVector )
        {
            variantVector.push_back( Variant( wrapper ) );
        }
        return variantVector;
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Variant Variant::fromVector( const std::vector<Variant>& variantVector )
{
    if ( variantVector.empty() ) return Variant();

    const Variant& variant = variantVector.front();

    Variant v = std::visit(
        [&variantVector]( auto&& arg ) -> Variant {
            using T = std::decay_t<decltype( arg )>;
            if constexpr ( !is_std_vector<T>::value )
            {
                return Variant::fromVectorT<T>( variantVector );
            }
            else
            {
                return Variant();
            }
        },
        variant.m_data.value() );

    return v;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<Variant> caf::Variant::toVectorT( const Variant& variant )
{
    if constexpr ( isVariantMember<T, InternalVariant>::value )
    {
        CAF_ASSERT( variant.canConvert<std::vector<T>>() );
        std::vector<Variant> vectorOfVariants;
        std::vector<T>       vectorOfValues = variant.value<std::vector<T>>();
        for ( T val : vectorOfValues )
        {
            vectorOfVariants.push_back( Variant( val ) );
        }
        return vectorOfVariants;
    }
    else
    {
        static_assert( false );
        return {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
Variant caf::Variant::fromVectorT( const std::vector<Variant>& variantVector )
{
    static_assert( !is_std_vector<T>::value );
    std::vector<T> vectorOfValues;
    for ( const Variant& variant : variantVector )
    {
        CAF_ASSERT( variant.canConvert<T>() );
        T value = variant.value<T>();
        vectorOfValues.push_back( value );
    }
    return Variant( vectorOfValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Variant::textValue() const
{
    if ( m_data.has_value() )
    {
        return std::visit(
            []( auto&& arg ) -> std::string {
                using T = std::decay_t<decltype( arg )>;
                std::stringstream ss;

                if constexpr ( is_std_vector<T>::value )
                {
                    for ( size_t i = 0; i < arg.size(); ++i )
                    {
                        if ( i > 0 ) ss << ", ";
                        ss << arg[i];
                    }
                    return ss.str();
                }
                else
                {
                    ss << arg;
                    return ss.str();
                }
            },
            m_data.value() );
    }
    else if ( m_serializableData )
    {
        return m_serializableData->textValue();
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Variant::operator<( const Variant& rhs ) const
{
    if ( m_data.has_value() && rhs.m_data.has_value() )
    {
        return m_data.value() < rhs.m_data.value();
    }
    else if ( m_data.has_value() || rhs.m_data.has_value() )
    {
        return m_data.has_value() < rhs.m_data.has_value();
    }

    if ( m_serializableData.has_value() && rhs.m_serializableData.has_value() )
    {
        return m_serializableData.value() < rhs.m_serializableData.value();
    }
    else if ( m_serializableData.has_value() != rhs.m_serializableData.has_value() )
    {
        return m_serializableData.has_value() < rhs.m_serializableData.has_value();
    }
    return false;
}
