#pragma once

namespace caffa
{
//--------------------------------------------------------------------------------------------------
/// This method is supposed to be the interface for the implementation of UI editors to set values into
/// the field. The data to set must be encapsulated in a Variant.
/// This method triggers Object::fieldChangedByUi() and Object::updateConnectedEditors(), an thus
/// makes the application and the UI aware of the change.
///
/// Note : If the field has m_optionEntryCache the interface is _index-based_. The Variant must contain
///        an UInt representing the index to the option selected by the user interface.
///
//--------------------------------------------------------------------------------------------------

template <typename FieldType>
void caffa::FieldUiCap<FieldType>::setValueFromUiEditor( const Variant& uiValue )
{
    Variant oldUiBasedVariant = toUiBasedVariant();

    typename FieldType::FieldDataType value;
    UiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant( uiValue, value );
    m_field->setValue( value );

    Variant newUiBasedVariant = toUiBasedVariant();

    this->notifyFieldChanged( oldUiBasedVariant, newUiBasedVariant );
}

//--------------------------------------------------------------------------------------------------
/// Extracts a Variant representation of the data in the field to be used in the UI.
//--------------------------------------------------------------------------------------------------

template <typename FieldType>
Variant caffa::FieldUiCap<FieldType>::uiValue() const
{
    return toUiBasedVariant();
}

//--------------------------------------------------------------------------------------------------
/// Returns the option values that is to be displayed in the UI for this field.
/// This method calls the virtual Object::calculateValueOptions to get the list provided from the
/// application.
//--------------------------------------------------------------------------------------------------

template <typename FieldType>
std::deque<OptionItemInfo> caffa::FieldUiCap<FieldType>::valueOptions( bool* useOptionsOnly ) const
{
    std::deque<OptionItemInfo> options;

    // First check if the owner Object has a value options specification. If it has, we use it.
    if ( m_field->ownerObject() )
    {
        options = uiObj( m_field->ownerObject() )->calculateValueOptions( this->m_field, useOptionsOnly );
    }

    // If we got no options, use the options defined by the type. Normally only caffa::AppEnum type

    if ( options.empty() )
    {
        options =
            UiFieldSpecialization<typename FieldType::FieldDataType>::valueOptions( useOptionsOnly, m_field->value() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
Variant caffa::FieldUiCap<FieldType>::toUiBasedVariant() const
{
    return UiFieldSpecialization<typename FieldType::FieldDataType>::convert( m_field->value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
bool caffa::FieldUiCap<FieldType>::isVariantDataEqual( const Variant& oldUiBasedVariant, const Variant& newUiBasedVariant ) const
{
    return ValueFieldSpecialization<typename FieldType::FieldDataType>::isEqual( oldUiBasedVariant, newUiBasedVariant );
}

} // End of namespace caffa
