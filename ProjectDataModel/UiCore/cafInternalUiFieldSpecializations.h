#pragma once

#include "cafObjectHandle.h"
#include "cafPointer.h"
#include "cafValueFieldSpecializations.h"
#include "cafVariant.h"

#include <list>
#include <string>
#include <type_traits>

namespace caffa
{
template <typename T>
class DataValueField;
template <typename T>
class Pointer;
template <typename T>
class AppEnum;

//==================================================================================================
/// Partial specialization for Field< Pointer<T> >
///
/// Will package the Pointer<T> into Variant as Pointer<Object>
/// Needed to support arbitrary types in Pointer without
/// havning to declare everything Q_DECLARE_METATYPE()
/// Also introduces the need for a isEqual() method, as this was the first
/// custom type embedded in Variant
//==================================================================================================

template <typename T>
class UiFieldSpecialization<Pointer<T>>
{
public:
    static Variant convert( const Pointer<T>& value ) { return Variant( Pointer<ObjectHandle>( value.rawPtr() ) ); }

    static Variant convertToUiVariant( const Pointer<T>& value ) { return convert( value ); }

    static void setFromVariant( const Variant& variantValue, Pointer<T>& value )
    {
        value.setRawPtr( variantValue.value<Pointer<ObjectHandle>>().rawPtr() );
    }

    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue.value<Pointer<ObjectHandle>>() == variantValue2.value<Pointer<ObjectHandle>>();
    }

    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const Pointer<T>& )
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
/// Partial specialization for Field<  caffa::AppEnum<T> >
//==================================================================================================

template <typename T>
class UiFieldSpecialization<caffa::AppEnum<T>>
{
public:
    /// Convert the field value into a Variant
    static Variant convert( const caffa::AppEnum<T>& value ) { return Variant( value ); }

    static Variant convertToUiVariant( const caffa::AppEnum<T>& value ) { return convert( value ); }

    /// Set the field value from a Variant
    static void setFromVariant( const Variant& variantValue, caffa::AppEnum<T>& value )
    {
        CAFFA_ASSERT( variantValue.canConvert<caffa::AppEnum<T>>() );
        value = variantValue.value<caffa::AppEnum<T>>();
    }

    static bool isDataElementEqual( const Variant& variantValue, const Variant& variantValue2 )
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly, const caffa::AppEnum<T>& )
    {
        if ( useOptionsOnly ) *useOptionsOnly = true;

        std::deque<OptionItemInfo> optionList;

        for ( size_t i = 0; i < caffa::AppEnum<T>::size(); ++i )
        {
            optionList.push_back( OptionItemInfo( caffa::AppEnum<T>::uiTextFromIndex( i ), caffa::AppEnum<T>::fromIndex( i ) ) );
        }

        return optionList;
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<caffa::AppEnum<T>>& field, std::vector<ObjectHandle*>* objects ) {}
};

} // End namespace caffa
