#pragma once

#include "cafAppEnum.h"
#include "cafPointer.h"
#include "cafVariant.h"

#include <assert.h>
#include <type_traits>
#include <vector>

namespace caffa
{
//==================================================================================================
/// A proxy class that implements the generic Variant interface for a field
///
/// This class collects methods that need specialization when introducing a new type in a Field.
/// Having those methods in a separate class makes it possible to "partially specialize" the methods
/// for container classes etc. since partial specialization of template functions is not C++ as of yet.
///
/// When introducing a new type in a Field, you might need to implement a (partial)specialization
/// of this class.
//==================================================================================================

template <typename T>
class ValueFieldSpecialization
{
public:
    /// Convert the field value into a Variant
    static Variant convert( const T& value ) { return Variant( value ); }

    /// Set the field value from a Variant
    static void setFromVariant( const Variant& variantValue, T& value ) { value = variantValue.value<T>(); }

    static bool isEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue == variantValue2;
    }
};

//==================================================================================================
/// Partial specialization for caffa::AppEnum
//==================================================================================================
template <typename T>
class ValueFieldSpecialization<caffa::AppEnum<T>>
{
public:
    static Variant convert( const caffa::AppEnum<T>& value ) { return Variant( value ); }

    static void setFromVariant( const Variant& variantValue, caffa::AppEnum<T>& value )
    {
        value = variantValue.value<caffa::AppEnum<T>>();
    }

    static bool isEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue == variantValue2;
    }
};

//==================================================================================================
/// Partial specialization for caffa::Pointer<T>
/// User must use PtrField or ChildField
//==================================================================================================
template <typename T>
class ValueFieldSpecialization<Pointer<T>>
{
public:
    static Variant convert( const Pointer<T>& value ) { return Variant( Pointer<ObjectHandle>( value.rawPtr() ) ); }

    static void setFromVariant( const Variant& variantValue, caffa::Pointer<T>& value )
    {
        value.setRawPtr( variantValue.value<Pointer<ObjectHandle>>().rawPtr() );
    }

    static bool isEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue.value<Pointer<ObjectHandle>>() == variantValue2.value<Pointer<ObjectHandle>>();
    }
};

} // End of namespace caffa
