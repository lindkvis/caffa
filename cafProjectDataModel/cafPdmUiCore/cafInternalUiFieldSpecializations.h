#pragma once

#include "cafObjectHandle.h"
#include "cafPdmPointer.h"
#include "cafValueFieldSpecializations.h"
#include "cafVariant.h"

#include <list>
#include <string>
#include <type_traits>

namespace caf
{
template <typename T>
class DataValueField;
template <typename T>
class PdmPointer;
template <typename T>
class AppEnum;

//==================================================================================================
/// Partial specialization for Field< PdmPointer<T> >
///
/// Will package the PdmPointer<T> into Variant as PdmPointer<Object>
/// Needed to support arbitrary types in PdmPointer without
/// havning to declare everything Q_DECLARE_METATYPE()
/// Also introduces the need for a isEqual() method, as this was the first
/// custom type embedded in Variant
//==================================================================================================

template <typename T>
class UiFieldSpecialization<PdmPointer<T>>
{
public:
    static Variant convert( const PdmPointer<T>& value )
    {
        return Variant( PdmPointer<ObjectHandle>( value.rawPtr() ) );
    }

    static Variant convertToUiVariant( const PdmPointer<T>& value ) { return convert( value ); }

    static void setFromVariant( const Variant& variantValue, PdmPointer<T>& value )
    {
        value.setRawPtr( variantValue.value<PdmPointer<ObjectHandle>>().rawPtr() );
    }

    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue.value<PdmPointer<ObjectHandle>>() == variantValue2.value<PdmPointer<ObjectHandle>>();
    }

    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const PdmPointer<T>& )
    {
        return std::deque<OptionItemInfo>();
    }
};

//==================================================================================================
/// Partial specialization for Field< std::list<T> >
//==================================================================================================

template <typename T>
class UiFieldSpecialization<std::list<T>>
{
public:
    /// Convert the field value into a Variant
    static Variant convert( const std::list<T>& value )
    {
        std::vector<Variant> returnList;
        for ( T item : value )
        {
            returnList.push_back( Variant( item ) );
        }
        return Variant::fromVector( returnList );
    }

    /// Convert the field value into a Variant
    static Variant convertToUiVariant( const std::list<T>& value )
    {
        std::vector<Variant> returnList;
        for ( T item : value )
        {
            returnList.push_back( UiFieldSpecialization<T>::convertToUiVariant( item ) );
        }
        return Variant::fromVector( returnList );
    }

    /// Set the field value from a Variant
    static void setFromVariant( const Variant& variantValue, std::list<T>& value )
    {
        if ( variantValue.isVector() )
        {
            auto variantVector = variantValue.toVector();

            value.clear();
            for ( auto variantItem : variantVector )
            {
                value.push_back( variantItem.value<T>() );
            }
        }
    }

    /// Operates on scalar content T value of the std::list<T>
    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return ValueFieldSpecialization<T>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const std::list<T>& )
    {
        return std::deque<OptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<std::list<T>>&, std::vector<ObjectHandle*>* ) {}
};

//==================================================================================================
/// Partial specialization for Field< std::vector<T> >
//==================================================================================================

template <typename T>
class UiFieldSpecialization<std::vector<T>>
{
public:
    /// Convert the field value into a Variant
    static Variant convert( const std::vector<T>& value )
    {
        std::vector<Variant> returnList;
        for ( T item : value )
        {
            returnList.push_back( Variant( item ) );
        }
        return Variant::fromVector( returnList );
    }

    /// Convert the field value into a Variant
    static Variant convertToUiVariant( const std::vector<T>& value )
    {
        std::vector<Variant> returnList;
        for ( T item : value )
        {
            returnList.push_back( UiFieldSpecialization<T>::convertToUiVariant( item ) );
        }
        return Variant::fromVector( returnList );
    }

    /// Set the field value from a Variant
    static void setFromVariant( const Variant& variantValue, std::vector<T>& value )
    {
        if ( variantValue.isVector() )
        {
            auto variantVector = variantValue.toVector();

            value.clear();
            for ( auto variantItem : variantVector )
            {
                value.push_back( variantItem.value<T>() );
            }
        }
    }

    /// Operates on scalar content T value of the std::vector<T>
    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return ValueFieldSpecialization<T>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const std::vector<T>& )
    {
        return std::deque<OptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<std::vector<T>>& field, std::vector<ObjectHandle*>* objects ) {}
};

//==================================================================================================
/// Partial specialization for Field<  caf::AppEnum<T> >
//==================================================================================================

template <typename T>
class UiFieldSpecialization<caf::AppEnum<T>>
{
public:
    /// Convert the field value into a Variant
    static Variant convert( const caf::AppEnum<T>& value ) { return Variant( value ); }

    static Variant convertToUiVariant( const caf::AppEnum<T>& value ) { return convert( value ); }

    /// Set the field value from a Variant
    static void setFromVariant( const Variant& variantValue, caf::AppEnum<T>& value )
    {
        CAF_ASSERT( variantValue.canConvert<caf::AppEnum<T>>() );
        value = variantValue.value<caf::AppEnum<T>>();
    }

    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const caf::AppEnum<T>& )
    {
        if ( useOptionsOnly ) *useOptionsOnly = true;

        std::deque<OptionItemInfo> optionList;

        for ( size_t i = 0; i < caf::AppEnum<T>::size(); ++i )
        {
            optionList.push_back( OptionItemInfo( caf::AppEnum<T>::uiTextFromIndex( i ), caf::AppEnum<T>::fromIndex( i ) ) );
        }

        return optionList;
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<caf::AppEnum<T>>& field, std::vector<ObjectHandle*>* objects ) {}
};

} // End namespace caf
