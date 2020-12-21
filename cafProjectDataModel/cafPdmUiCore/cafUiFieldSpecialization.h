#pragma once

#include "cafUiItem.h"
#include "cafVariant.h"

#include <deque>
#include <vector>

namespace caf
{
template <typename T>
class DataValueField;
class ObjectHandle;

//==================================================================================================
/// A proxy class that implements the Gui interface of fields
///
/// This class collects methods that need specialization when introducing a new type in a Field.
/// Having those methods in a separate class makes it possible to "partially specialize" the methods
/// for container classes etc. since partial specialization of template functions is not C++ as of yet.
///
/// When introducing a new type in a Field, you might need to implement a (partial)specialization
/// of this class.
//==================================================================================================

template <typename T>
class UiFieldSpecialization
{
public:
    /// Convert the field value into a regular Variant
    static Variant convert( const T& value ) { return Variant( value ); }

    static Variant convertToUiVariant( const T& value )
    {
        if constexpr ( std::is_same<T, std::string>::value )
        {
            return Variant( value );
        }
        else
        {
            std::stringstream ss;
            ss << value;
            return Variant( ss.str() );
        }
    }

    /// Set the field value from a Variant
    static void setFromVariant( const Variant& variantValue, T& value )
    {
        if constexpr ( !std::is_same<T, std::string>::value )
        {
            if ( variantValue.canConvert<std::string>() )
            {
                std::string       strValue = variantValue.value<std::string>();
                std::stringstream ss( strValue );
                ss >> value;
            }
            else
            {
                value = variantValue.value<T>();
            }
        }
        else
        {
            value = variantValue.value<T>();
        }
    }

    /// Check equality between Variants that carries a Field Value.
    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static std::deque<caf::OptionItemInfo> valueOptions( bool* useOptionsOnly, const T& )
    {
        return std::deque<caf::OptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<T>&, std::vector<ObjectHandle*>* ) {}
};
} // End of namespace caf

#include "cafInternalUiFieldSpecializations.h"
