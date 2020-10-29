#pragma once

#include <QList>
#include <QVariant>

#include <vector>

namespace caf
{
template <typename T>
class DataValueField;
class PdmOptionItemInfo;
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
class PdmUiFieldSpecialization
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const T& value ) { return QVariant::fromValue( value ); }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, T& value ) { value = variantValue.value<T>(); }

    /// Check equality between QVariants that carries a Field Value.
    /// The == operator will normally work, but does not support custom types in the QVariant
    /// See http://qt-project.org/doc/qt-4.8/qvariant.html#operator-eq-eq-64
    /// This is needed for the lookup regarding OptionValues
    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        if ( variantValue.type() == QVariant::UserType )
        {
            return ( variantValue.value<T>() == variantValue2.value<T>() );
        }
        else
        {
            return variantValue == variantValue2;
        }
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const T& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible Object pointed to by a field
    static void childObjects( const DataValueField<T>&, std::vector<ObjectHandle*>* ) {}
};
} // End of namespace caf

#include "cafInternalUiFieldSpecializations.h"
