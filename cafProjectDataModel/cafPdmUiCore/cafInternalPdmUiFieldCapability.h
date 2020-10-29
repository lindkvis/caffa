#pragma once

#include "cafFieldUiCapability.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"

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
    QVariant                 uiValue() const override;
    void                     setValueFromUiEditor( const QVariant& uiValue ) override;
    QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const override;

    QVariant toUiBasedQVariant() const override;

private:
    bool isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const override;

    mutable QList<PdmOptionItemInfo> m_optionEntryCache;

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
    QVariant                 uiValue() const override { return QVariant(); }
    void                     setValueFromUiEditor( const QVariant& uiValue ) override {}
    QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const override { return QList<PdmOptionItemInfo>(); }

    QVariant toUiBasedQVariant() const override { return QVariant(); }
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
    QVariant                 uiValue() const override { return QVariant(); }
    void                     setValueFromUiEditor( const QVariant& uiValue ) override {}
    QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const override { return QList<PdmOptionItemInfo>(); }

    QVariant toUiBasedQVariant() const override { return QVariant(); }
};

template <typename FieldType>
void AddUiCapabilityToField( FieldType* field )
{
    if ( field->template capability<FieldUiCap<FieldType>>() == NULL )
    {
        new FieldUiCap<FieldType>( field, true );
    }
}

} // End of namespace caf

#include "cafInternalPdmUiFieldCapability.inl"
