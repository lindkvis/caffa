#pragma once

#include "cafFieldUiCapability.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafVariant.h"

#include <deque>

namespace caf
{
template <typename FieldType>
class FieldUiCap : public FieldUiCapability
{
public:
    FieldUiCap( FieldType* field, bool giveOwnership )
        : FieldUiCapability( field, giveOwnership )
    {
        m_field = field;
    }

    // Gui generalized interface
public:
    Variant                    uiValue() const override;
    void                       setValueFromUiEditor( const Variant& uiValue ) override;
    std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly ) const override;

    Variant toUiBasedVariant() const override;

private:
    bool isVariantDataEqual( const Variant& oldUiBasedVariant, const Variant& newUiBasedVariant ) const override;

private:
    FieldType* m_field;
};

//
// Specialization for ChildFields to do nothing towards GUI
//
template <typename DataType>
class FieldUiCap<ChildField<DataType*>> : public FieldUiCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldUiCap( FieldType* field, bool giveOwnership )
        : FieldUiCapability( field, giveOwnership )
    {
    }

    // Gui generalized interface
public:
    Variant                    uiValue() const override { return Variant(); }
    void                       setValueFromUiEditor( const Variant& uiValue ) override {}
    std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly ) const override
    {
        return std::deque<OptionItemInfo>();
    }

    Variant toUiBasedVariant() const override { return Variant(); }
};

//
// Specialization for ChildArrayFields to do nothing towards GUI
//
template <typename DataType>
class FieldUiCap<ChildArrayField<DataType*>> : public FieldUiCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldUiCap( FieldType* field, bool giveOwnership )
        : FieldUiCapability( field, giveOwnership )
    {
    }

    // Gui generalized interface
public:
    Variant                    uiValue() const override { return Variant(); }
    void                       setValueFromUiEditor( const Variant& uiValue ) override {}
    std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly ) const override
    {
        return std::deque<OptionItemInfo>();
    }

    Variant toUiBasedVariant() const override { return Variant(); }
};

template <typename FieldType>
void AddUiCapabilityToField( FieldType* field )
{
    if ( field->template capability<FieldUiCap<FieldType>>() == nullptr )
    {
        new FieldUiCap<FieldType>( field, true );
    }
}

} // End of namespace caf

#include "cafInternalUiFieldCapability.inl"
