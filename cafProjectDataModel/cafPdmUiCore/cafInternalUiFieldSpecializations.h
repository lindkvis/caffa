#pragma once

#include "cafObjectHandle.h"
#include "cafPdmPointer.h"
#include "cafValueFieldSpecializations.h"

#include <QStringList>

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
/// Will package the PdmPointer<T> into QVariant as PdmPointer<Object>
/// Needed to support arbitrary types in PdmPointer without
/// havning to declare everything Q_DECLARE_METATYPE()
/// Also introduces the need for a isEqual() method, as this was the first
/// custom type embedded in QVariant
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization<PdmPointer<T>>
{
public:
    static QVariant convert( const PdmPointer<T>& value )
    {
        return QVariant::fromValue( PdmPointer<ObjectHandle>( value.rawPtr() ) );
    }

    static void setFromVariant( const QVariant& variantValue, PdmPointer<T>& value )
    {
        value.setRawPtr( variantValue.value<PdmPointer<ObjectHandle>>().rawPtr() );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return variantValue.value<PdmPointer<ObjectHandle>>() == variantValue2.value<PdmPointer<ObjectHandle>>();
    }

    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const PdmPointer<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }
};

//==================================================================================================
/// Partial specialization for Field< std::list<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization<std::list<T>>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const std::list<T>& value )
    {
        QList<QVariant>                       returnList;
        typename std::list<T>::const_iterator it;
        for ( it = value.begin(); it != value.end(); ++it )
        {
            returnList.push_back( QVariant( *it ) );
        }
        return returnList;
    }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, std::list<T>& value )
    {
        if ( variantValue.canConvert<QList<QVariant>>() )
        {
            value.clear();
            QList<QVariant> lst = variantValue.toList();
            int             i;
            for ( i = 0; i < lst.size(); ++i )
            {
                value.push_back( lst[i].value<T>() );
            }
        }
    }

    /// Operates on scalar content T value of the std::list<T>
    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return ValueFieldSpecialization<T>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const std::list<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<std::list<T>>&, std::vector<ObjectHandle*>* ) {}
};

//==================================================================================================
/// Partial specialization for Field< std::vector<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization<std::vector<T>>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const std::vector<T>& value )
    {
        return ValueFieldSpecialization<std::vector<T>>::convert( value );
    }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, std::vector<T>& value )
    {
        return ValueFieldSpecialization<std::vector<T>>::setFromVariant( variantValue, value );
    }

    /// Operates on scalar content T value of the std::vector<T>
    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return ValueFieldSpecialization<T>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const std::vector<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<std::vector<T>>& field, std::vector<ObjectHandle*>* objects )
    {
    }
};

//==================================================================================================
/// Partial specialization for Field<  caf::AppEnum<T> >
//==================================================================================================

template <typename T>
class PdmUiFieldSpecialization<caf::AppEnum<T>>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const caf::AppEnum<T>& value )
    {
        T enumVal = value;
        // Explicit cast to an int for storage in a QVariant. This allows the use of enum class instead of enum
        return QVariant( static_cast<int>( enumVal ) );
    }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, caf::AppEnum<T>& value )
    {
        value = static_cast<T>( variantValue.toInt() );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return variantValue == variantValue2;
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const caf::AppEnum<T>& )
    {
        if ( useOptionsOnly ) *useOptionsOnly = true;

        QList<PdmOptionItemInfo> optionList;

        for ( size_t i = 0; i < caf::AppEnum<T>::size(); ++i )
        {
            T enumVal = caf::AppEnum<T>::fromIndex( i );
            optionList.push_back( PdmOptionItemInfo( caf::AppEnum<T>::uiTextFromIndex( i ), static_cast<int>( enumVal ) ) );
        }

        return optionList;
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<caf::AppEnum<T>>& field, std::vector<ObjectHandle*>* objects )
    {
    }
};

//==================================================================================================
/// Partial specialization for FilePath
//==================================================================================================

template <>
class PdmUiFieldSpecialization<caf::FilePath>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const caf::FilePath& value )
    {
        return ValueFieldSpecialization<caf::FilePath>::convert( value );
    }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, caf::FilePath& value )
    {
        return ValueFieldSpecialization<caf::FilePath>::setFromVariant( variantValue, value );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return ValueFieldSpecialization<caf::FilePath>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const caf::FilePath& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<caf::FilePath>& field, std::vector<ObjectHandle*>* objects ) {}
};

} // End namespace caf
